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
#include "drivers/i2c.h"

#include <avr/io.h>
#include <util/twi.h>

#include "avrx/avrx.h"
#include "cdp_control.h"
#include "drivers/timer.h"

namespace cdp {

IOREGISTER8(TWCR);

using Timeout = Timer1::Timeout<Timer1::CHANNEL_B>;

void I2C::Init()
{
  TWSR = 0;
  TWBR = ((F_CPU / SCL_FREQ) - 16) / 2;

  TWCRRegister::Write<TWEN, TWINT>();

  Timeout::SetMs(kI2CTimeoutMs);
}

bool I2C::Start(uint8_t address)
{
  TWCRRegister::Write<TWINT, TWSTA, TWEN>();
  Timeout::Arm();
  while (!(TWCR & _BV(TWINT))) {
    if (Timeout::timeout()) return false;
  }

  auto twst = TW_STATUS & 0xf8;  // mask prescaler
  if ((twst != TW_START) && (twst != TW_REP_START)) return false;

  TWDR = address;
  TWCRRegister::Write<TWINT, TWEN>();

  while (!(TWCR & _BV(TWINT))) {}

  twst = TW_STATUS & 0xf8;
  if ((twst != TW_MT_SLA_ACK) && (twst != TW_MR_SLA_ACK)) return false;

  return true;
}

bool I2C::Write(uint8_t data)
{
  TWDR = data;
  TWCRRegister::Write<TWINT, TWEN>();
  while (!(TWCR & _BV(TWINT))) {}

  auto twst = TW_STATUS & 0xf8;
  if (twst != TW_MT_DATA_ACK) return false;

  return true;
}

uint8_t I2C::ReadAck()
{
  TWCRRegister::Write<TWINT, TWEN, TWEA>();
  while (!(TWCR & _BV(TWINT))) {}

  return TWDR;
}

uint8_t I2C::ReadNack()
{
  TWCRRegister::Write<TWINT, TWEN>();
  while (!(TWCR & _BV(TWINT))) {}

  return TWDR;
}

bool I2C::Stop()
{
  TWCRRegister::Write<TWEN, TWINT, TWSTO>();
  Timeout::Arm();
  while (TWCR & _BV(TWSTO)) {
    if (Timeout::timeout()) return false;
  }

  return true;
}

}  // namespace cdp
