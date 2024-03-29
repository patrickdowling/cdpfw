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
#include "serial_port.h"

#include <avr/interrupt.h>
#include <avr/pgmspace.h>

#include "gpio.h"

namespace cdp {

// TODO We need a bulk copy for ring buffers, or a better way to queue messages
// TODO This is inconsistent with using bits or nice wrappers
//
// NOTE
// TXD/PCINT17 – Port D, Bit 1
// TXD, transmit Data (data output pin for the USART). When the USART transmitter is enabled, this
// pin is configured as an output regardless of the value of DDD1. PCINT17: Pin change interrupt
// source 17. The PD1 pin can serve as an external interrupt source.
// RXD/PCINT16 – Port D, Bit 0
// RXD, Receive Data (data input pin for the USART). When the USART receiver is enabled this pin is
// configured as an input regardless of the value of DDD0. When the USART forces this pin to be an
// input, the pull-up can still be controlled by the PORTD0 bit.

static volatile uint8_t rx_errors_ = 0;

#define BAUD SERIAL_BAUD
#include <util/setbaud.h>

IOREGISTER8(UCSR0A);
IOREGISTER8(UCSR0B);
IOREGISTER8(UCSR0C);

namespace usart0 {
using DRIE = avrx::RegisterBit<UCSR0BRegister, UDRIE0>;
using DRE = avrx::RegisterBit<UCSR0ARegister, UDRE0>;
using RXEN = avrx::RegisterBit<UCSR0BRegister, RXEN0>;
using TXEN = avrx::RegisterBit<UCSR0BRegister, TXEN0>;
using RXCIE = avrx::RegisterBit<UCSR0BRegister, RXCIE0>;
using U2X = avrx::RegisterBit<UCSR0ARegister, U2X0>;
}  // namespace usart0

void SerialPort::Init()
{
#if USE_2X
  usart0::U2X::set();
#endif
  UBRR0H = UBRRH_VALUE;
  UBRR0L = UBRRL_VALUE;

  // Enable: TX, RX (RX interrupt enabled later)
  UCSR0BRegister::SetBits<usart0::RXEN, usart0::TXEN>();

  gpio::RXD::OverridePullupEnable();

  // Set frame format 8N1
  UCSR0C = (3 << UCSZ00);
}

void SerialPort::EnableRx()
{
  usart0::RXCIE::set();
}

void SerialPort::Write(const char *buffer)
{
#ifdef ENABLE_USART_TX
  while (*buffer) { tx_buffer_::Push(*buffer++); }
  usart0::DRIE::set();
#else
  while (*buffer) {
    while (!usart0::DRE::value()) { }
    UDR0 = *buffer++;
  }
#endif
}

void SerialPort::WriteP(const char *buffer)
{
  auto c = pgm_read_byte(buffer++);
#ifdef ENABLE_USART_TX
  while (c) {
    tx_buffer_::Push(c);
    c = pgm_read_byte(buffer++);
  }
  usart0::DRIE::set();
#else
  while (c) {
    while (!usart0::DRE::value()) { }
    UDR0 = c;
    c = pgm_read_byte(buffer++);
  }
#endif
}

void SerialPort::WriteImmediateP(const char *buffer)
{
  auto c = pgm_read_byte(buffer++);
  while (c) {
    while (!usart0::DRE::value()) { }
    UDR0 = c;
    c = pgm_read_byte(buffer++);
  }
}

#ifdef ENABLE_USART_TX
/*static*/ void SerialPort::Tx()
{
  if (!tx_buffer_::empty()) {
    UDR0 = tx_buffer_::Pop();
  } else {
    usart0::DRIE::set();
  }
}
#endif

}  // namespace cdp

using namespace cdp;

ISR(USART_RX_vect)
{
  uint8_t status = UCSR0A;
  char data = UDR0;

  if (status & ((1 << FE0) | (1 << UPE0) | (1 << DOR0))) {
    ++rx_errors_;
  } else {
    SerialPort::Rx(data);
  }
}

#ifdef ENABLE_USART_TX
ISR(USART_UDRE_vect)
{
  SerialPort::Tx();
}
#endif

