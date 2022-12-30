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
#ifndef SRC_4392_H_
#define SRC_4392_H_

#include <stdint.h>

namespace cdp {

struct SRCState;

class SRC4392 {
public:
  static constexpr uint8_t kSampleRate = 96;
  static constexpr uint8_t kI2CAddress = 0x70;

  static constexpr uint8_t REGISTER_INC = 0x80;
  enum Register : uint8_t {
    RESET = 0x01,
    PORTA_CONTROL = 0x03,
    PORTB_CONTROL = 0x05,
    TRANSMITTER_CONTROL = 0x07,
    RECEIVER_CONTROL = 0x0d,
    GPO1 = 0x1b,
    GPO2 = 0x1c,
    GPO3 = 0x1d,
    GPO4 = 0x1e,
    SRC_CONTROL = 0x2d,
    SRC_CONTROL_ATT_L = 0x30,
    SRC_CONTROL_ATT_R = 0x31,
    SRC_RATIO_READBACK = 0x32,
    PAGE_SELECTION = 0x7f
  };

  enum Bits : uint8_t {
    SRC_MUTE = 0x10,
  };

  // Initial init
  static void Init();

  static void Update(const SRCState &state);

  static uint16_t ReadRatio();

private:
};

}  // namespace cdp

#endif  // SRC_4392_H_

