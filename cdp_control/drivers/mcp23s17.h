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
#ifndef DRIVERS_MCP23S17_H_
#define DRIVERS_MCP23S17_H_

#include <stdint.h>

#include "drivers/gpio.h"
#include "drivers/spi.h"

namespace cdp {
class MCP23S17 {
public:
  using CS = gpio::MCP_CS;

  enum class PORT : uint8_t { A, B };

  // There's two operating modes for the registers that is determined by IOCON.BANK.
  // This assumes IOCON.BANK=0, so the control registers for the A/B registers are paired.
  enum REGISTER : uint8_t {
    IODIR = 0x00,
    IPOL,
    GPINTEN,
    DEFVAL,
    INTCON,
    IOCON,
    GPPU,
    INTF,
    INTCAP,
    GPIO,
    OLAT,
    NUM_REGISTERS
  };
  static_assert(11 == NUM_REGISTERS, "Invalid number of registers");

  static constexpr uint8_t kDeviceAddress = 0x40;
  static constexpr uint8_t kControlByteRead = 0x01;

  static inline constexpr uint8_t MapRegisterAddress(PORT port, REGISTER reg)
  {
    return reg * 2 + (int)port;
  }

  enum IOCON : uint8_t {
    UNUSED = 0,
    INTPOL = 0x1 << 1,
    ODR = 0x1 << 2,
    HAEN = 0x1 << 3,
    DISSLW = 0x1 << 4,
    SEQOP = 0x1 << 5,
    MIRROR = 0x1 << 6,
    BANK = 0x1 << 7
  };

  static void Init(uint8_t output_value);

  static void WritePortRegister(PORT port, REGISTER reg, uint8_t value)
  {
    const uint8_t data[] = {kDeviceAddress, MCP23S17::MapRegisterAddress(port, reg), value};

    avrx::ScopedPulse<CS, avrx::GPIO_RESET> cs;
    SPI::Write(data, sizeof(data));
  }

  static uint8_t ReadPortRegister(PORT port, REGISTER reg)
  {
    const uint8_t data[] = {kDeviceAddress | kControlByteRead,
                            MCP23S17::MapRegisterAddress(port, reg), 0xff};

    avrx::ScopedPulse<CS, avrx::GPIO_RESET> cs;
    return SPI::Read8(data, sizeof(data));
  }
};

}  // namespace cdp

#endif  // DRIVERS_MCP23S17_H_

