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
// We could also use the timer interrupt (and disable it when its triggered)?

PROGMEM const char dsa_status_ok[] = "OK";
PROGMEM const char dsa_status_timeout[] = "TIMEOUT";
PROGMEM const char dsa_status_err[] = "ERR";
PROGMEM const char *const dsa_status_strings[] = {
    dsa_status_ok,
    dsa_status_timeout,
    dsa_status_err,
};

const char *DSA::to_pstring(DSA_STATUS dsa_status)
{
  return (PGM_P)pgm_read_word(&(dsa_status_strings[dsa_status]));
}

using namespace gpio;
using Timeout = Timer1::Timeout<Timer1::CHANNEL_A>;

struct ResetOnExit {
  ~ResetOnExit() { DSA::ResetPinState(); }
};

DSA::Message DSA::last_response_ = DSA::INVALID_MESSAGE;

void DSA::Init()
{
  ResetPinState();
  Timeout::SetMs(kI2CTimeoutMs);
}

void DSA::ResetPinState()
{
  // TODO These are all on the same port, so these operations could be merged
  avrx::InitPins<DSA_DATA, DSA_STROBE, DSA_ACK>();
}

DSA::DSA_STATUS DSA::Receive()
{
  ResetOnExit reset_on_exit{};

  last_response_ = INVALID_MESSAGE;
  Message message = INVALID_MESSAGE;

  // Synchronization
  DSA_ACK::SetOutputMode();
  Timeout::Arm();
  DSA_ACK::reset();
  while (!DSA_DATA::value()) {
    if (Timeout::timeout()) { return STATUS_TIMEOUT; }
  }
  DSA_ACK::set();

  // Transmission
  Timeout::Arm();
  uint16_t mask = 0x8000;
  while (mask) {
    while (DSA_STROBE::value()) {
      if (Timeout::timeout()) { return STATUS_TIMEOUT; }
    }
    if (DSA_DATA::value()) message |= mask;

    DSA_ACK::reset();
    while (!DSA_STROBE::value()) {
      if (Timeout::timeout()) { return STATUS_TIMEOUT; }
    }
    DSA_ACK::set();
    mask >>= 1;
  }

  // Acknowledge
  DSA_DATA::SetOutputMode();
  DSA_STROBE::SetOutputMode();
  DSA_ACK::SetInputMode(true);

  Timeout::Arm();
  while (DSA_ACK::value()) {
    if (Timeout::timeout()) return STATUS_TIMEOUT;
  }

  if (mask) DSA_DATA::reset();
  DSA_STROBE::reset();

  while (!DSA_ACK::value()) {
    if (Timeout::timeout()) return STATUS_TIMEOUT;
  }
  DSA_DATA::set();
  DSA_STROBE::set();

  if (!mask) {
    last_response_ = message;
    return STATUS_OK;
  } else {
    last_response_ = INVALID_MESSAGE;
    return STATUS_ERR;
  }
}

DSA::DSA_STATUS DSA::Transmit(Message message)
{
  ResetOnExit reset_on_exit{};

  // Synchronization
  DSA_DATA::SetOutputMode();

  Timeout::Arm();
  DSA_DATA::reset();
  while (DSA_ACK::value()) {
    if (Timeout::timeout()) return STATUS_TIMEOUT;
  }
  DSA_DATA::set();
  while (!DSA_ACK::value()) {
    if (Timeout::timeout()) return STATUS_TIMEOUT;
  }

  // Data transmission
  DSA_STROBE::SetOutputMode();
  DSA_STROBE::set();

  Timeout::Arm();
  uint16_t mask = 0x8000;
  while (mask) {
    if (!(mask & message)) DSA_DATA::reset();

    DSA_STROBE::reset();
    while (DSA_ACK::value()) {
      if (Timeout::timeout()) { return STATUS_TIMEOUT; }
    }
    DSA_STROBE::set();
    DSA_DATA::set();
    while (!DSA_ACK::value()) {
      if (Timeout::timeout()) { return STATUS_TIMEOUT; }
    }
    mask >>= 1;
  }

  // Acknowledge
  DSA_STROBE::SetInputMode(true);
  DSA_DATA::SetInputMode(true);
  DSA_ACK::SetOutputMode();

  Timeout::Arm();
  DSA_ACK::reset();
  while (DSA_STROBE::value()) {
    if (Timeout::timeout()) return STATUS_TIMEOUT;
  }

  // In case of error, we might also just bail
  DSA_STATUS dsa_status = DSA_DATA::value() ? STATUS_OK : STATUS_ERR;

  DSA_ACK::set();
  while (!DSA_STROBE::value()) {
    if (Timeout::timeout()) return STATUS_TIMEOUT;
  }

  return dsa_status;
}

}  // namespace cdp
