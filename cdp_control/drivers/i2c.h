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
#ifndef DRIVERS_I2C_H_
#define DRIVERS_I2C_H_

#include <stdint.h>

namespace cdp {

class I2C {
public:
  static constexpr uint32_t SCL_FREQ = 100000UL;

  static void Init();
  static bool Stop();

  template <typename... Bytes>
  static bool WriteBytes(uint8_t address, Bytes&&... bytes)
  {
    bool success = Start(address << 1);
    if (success) { (Write(bytes), ...); }
    Stop();
    return success;
  }

//private:
  static bool Start(uint8_t address);
  static bool Write(uint8_t data);
  static uint8_t ReadAck();
  static uint8_t ReadNack();
};

}  // namespace cdp

#endif  // DRIVERS_I2C_H_
