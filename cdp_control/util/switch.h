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
#ifndef UTIL_SWITCH_H_
#define UTIL_SWITCH_H_

#include <stdint.h>
#include <avr/io.h>

namespace util {

template <uint8_t bit_index>
class Switch {
public:
  static constexpr uint8_t kBitMask = _BV(bit_index);

  static inline void Update(uint8_t input_state)
  {
    uint8_t state = state_ << 1;
    if (input_state & kBitMask) state |= 0x1;
    state_ = state;
  }

  static constexpr uint8_t id() { return bit_index; }
  static inline bool just_pressed() { return 0x80 == state_; }
  static inline bool just_released() { return 0x7f == state_; }

private:
  static uint8_t state_;
};

template <uint8_t bit_index>
uint8_t Switch<bit_index>::state_ = 0;


}  // namespace util

#endif  // UTIL_SWITCH_H_
