// cdpfw
// Copyright (C) 2022 Patrick Dowling (pld@gurkenkiste.com)
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//
#include "dsa.h"

#include <avr/pgmspace.h>
#include <stdio.h>

#include "avrx/macros.h"
#include "cdp_control.h"
#include "drivers/serial_port.h"
#include "drivers/timer.h"

namespace cdp {

// Does it make sense to trigger on falling edge of DSA_DATA, or DSA_ACK?
// They will share the same interrupt though, and this might conflict with RC5 (although it's on
// D3).
//
// DSA_DATA = PC2 = PCINT10
// DSA_STROBE = PC3 = PCINT11
//
// NOTE The GPIOs are defined as inputs with pullups enabled

// Timeout = 250msec
// The 250ms timeout is for Tsyn (synchronization), Tcom (acknowledge timeout) and Ttfr (data
// transfer of all bits) according to DSA Interface Bus Protocol docs.
//
// We could also use the timer interrupt (and disable it when its triggered)?

PROGMEM_STRINGS3(dsa_status_strings, "OK", "TIMEOUT", "ERR");
const char *to_pstring(DSA::DSA_STATUS dsa_status)
{
  return (PGM_P)pgm_read_word(&(dsa_status_strings[dsa_status]));
}

using Timeout = Timer1::Timeout<Timer1::CHANNEL_A>;
using gpio::DSA_DATA;
using gpio::DSA_STROBE;
using gpio::DSA_ACK;

static void ResetPinState()
{
  // TODO These are all on the same port, so these operations could be merged
  avrx::InitPins<DSA_DATA, DSA_STROBE, DSA_ACK>();
}

struct ResetOnExit {
  ~ResetOnExit() { ResetPinState(); }
};

DSA::Message DSA::last_response_ = DSA::INVALID_MESSAGE;

void DSA::Init()
{
  ResetPinState();
  Timeout::SetMs(kDSATimeoutMS);
}

template <typename gpio>
static inline bool WaitForRisingEdge()
{
  while (!gpio::value()) {
    if (Timeout::timeout()) return false;
  }
  return true;
}

template <typename gpio>
static inline bool WaitForFallingEdge()
{
  while (gpio::value()) {
    if (Timeout::timeout()) return false;
  }
  return true;
}

DSA::DSA_STATUS DSA::Receive()
{
  ResetOnExit reset_on_exit{};

  last_response_ = INVALID_MESSAGE;
  Message message = INVALID_MESSAGE;

  // Synchronization
  Timeout::Arm();
  DSA_ACK::SetOutputMode();
  DSA_ACK::reset();
  if (!WaitForRisingEdge<DSA_DATA>()) return STATUS_TIMEOUT;
  DSA_ACK::set();

  // Transmission
  Timeout::Arm();
  uint16_t mask = 0x8000;
  while (mask) {
    if (!WaitForFallingEdge<DSA_STROBE>()) return STATUS_TIMEOUT;
    if (DSA_DATA::value()) message |= mask;

    DSA_ACK::reset();
    if (!WaitForRisingEdge<DSA_STROBE>()) return STATUS_TIMEOUT;
    DSA_ACK::set();
    mask >>= 1;
  }

  // Acknowledge
  DSA_DATA::SetOutputMode();
  DSA_STROBE::SetOutputMode();
  DSA_ACK::SetInputMode(true);

  Timeout::Arm();
  if (!WaitForFallingEdge<DSA_ACK>()) return STATUS_TIMEOUT;
  DSA_STROBE::reset();
  if (!WaitForRisingEdge<DSA_ACK>()) return STATUS_TIMEOUT;
  DSA_DATA::set();
  DSA_STROBE::set();

  last_response_ = message;
  return STATUS_OK;
}

DSA::DSA_STATUS DSA::Transmit(Message message)
{
  ResetOnExit reset_on_exit{};

  // Synchronization
  Timeout::Arm();
  DSA_DATA::SetOutputMode();
  DSA_DATA::reset();
  if (!WaitForFallingEdge<DSA_ACK>()) return STATUS_TIMEOUT;
  DSA_DATA::set();
  if (!WaitForRisingEdge<DSA_ACK>()) return STATUS_TIMEOUT;

  // Data transmission
  DSA_STROBE::SetOutputMode();
  DSA_STROBE::set();

  Timeout::Arm();
  uint16_t mask = 0x8000;
  while (mask) {
    if (!(mask & message)) DSA_DATA::reset();

    DSA_STROBE::reset();
    if (!WaitForFallingEdge<DSA_ACK>()) return STATUS_TIMEOUT;
    DSA_STROBE::set();
    DSA_DATA::set();
    if (!WaitForRisingEdge<DSA_ACK>()) return STATUS_TIMEOUT;
    mask >>= 1;
  }

  // Acknowledge
  DSA_STROBE::SetInputMode(true);
  DSA_DATA::SetInputMode(true);
  DSA_ACK::SetOutputMode();

  Timeout::Arm();
  DSA_ACK::reset();
  if (!WaitForFallingEdge<DSA_STROBE>()) return STATUS_TIMEOUT;

  // In case of error, we might also just bail
  DSA_STATUS dsa_status = DSA_DATA::value() ? STATUS_OK : STATUS_ERR;

  DSA_ACK::set();
  if (!WaitForRisingEdge<DSA_STROBE>()) return STATUS_TIMEOUT;

  return dsa_status;
}

}  // namespace cdp
