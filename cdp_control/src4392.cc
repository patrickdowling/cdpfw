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
#include "src4392.h"

#include <avr/pgmspace.h>
#include <string.h>

#include "drivers/i2c.h"
#include "drivers/serial_port.h"
#include "src_state.h"

namespace cdp {

// GPO1 = PCM1794A:CHSL = DF rolloff = sharp (0 = default)
// GPO2 = TAS3193 reset

// READ
// 0xb2 = 32 = SRC Input: Output Ratio
// 03 AC = 940/2048 * 96KHz = 44.0625

void SRC4392::Init()
{
  int success = 0;
  success += I2C::WriteBytes(kI2CAddress);
  success += I2C::WriteBytes(kI2CAddress, PAGE_SELECTION, 0x00);
  success += I2C::WriteBytes(kI2CAddress, RESET, 0x3f);  // power down false for all fields
  success += I2C::WriteBytes(kI2CAddress, REGISTER_INC | PORTA_CONTROL, 0x39, 0x01,
                             /*port b->*/ 0x01, 0x01);
  success += I2C::WriteBytes(kI2CAddress, REGISTER_INC | TRANSMITTER_CONTROL, 0x38, 0x00);
  success += I2C::WriteBytes(kI2CAddress, REGISTER_INC | RECEIVER_CONTROL, 0x08, 0x19,
                             /*receiver PLL*/ 0x22);
  success += I2C::WriteBytes(kI2CAddress, REGISTER_INC | SRC_CONTROL, SRC_MUTE | SOURCE_I2S, 0x00);

  success += I2C::WriteBytes(kI2CAddress, REGISTER_INC | SRC_CONTROL_ATT_L, 0xff, 0xff);

  success += I2C::WriteBytes(kI2CAddress, GPO2, 0x01);
  success += I2C::WriteBytes(kI2CAddress, GPO1, 0x00);

  success += I2C::WriteBytes(kI2CAddress, REGISTER_INC | TRANSMITTER_CONTROL, 0x38, 0x07);
  success += I2C::WriteBytes(kI2CAddress, GPO2, 0x00);  // TAS3193 reset

  SerialPort::PrintfP(PSTR("SRC4392::Init %d\r\n"), success);
}

void SRC4392::Update(const SRCState &state)
{
  if (I2C::WriteBytes(kI2CAddress, PAGE_SELECTION, 0x00)) {
    if (state.update_source())
      I2C::WriteBytes(kI2CAddress, REGISTER_INC | SRC_CONTROL,
                      state.source /*| state.mute ? SRC_MUTE : 0*/);

    if (state.update_attenuation())
      I2C::WriteBytes(kI2CAddress, REGISTER_INC | SRC_CONTROL_ATT_L, state.attenuation,
                      state.attenuation);
  }
}

uint16_t SRC4392::ReadRatio()
{
  uint8_t ratio[2] = {0xff, 0xff};
  if (I2C::Start(kI2CAddress << 1)) {
    I2C::Write(REGISTER_INC | SRC_RATIO_READBACK);
    I2C::Start((kI2CAddress << 1) | 0x1);
    ratio[0] = I2C::ReadAck();
    ratio[1] = I2C::ReadNack();
  }
  I2C::Stop();
  return (ratio[0] << 8) | (ratio[1]);
}

void SRC4392::CommandHandler(const char *cmd)
{
  if (!strcmp(cmd, "INIT")) {
    Init();
  } else if (!strcmp(cmd, "RATIO")) {
    ReadRatio();
  } else if (!strcmp(cmd, "PING")) {
    if (I2C::WriteBytes(kI2CAddress, PAGE_SELECTION, 0x00))
      SerialPort::PrintfP(PSTR("I2C OK\r\n"));
    else
      SerialPort::PrintfP(PSTR("I2C FAIL\r\n"));
  }
}

}  // namespace cdp
