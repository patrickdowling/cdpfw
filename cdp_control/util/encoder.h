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
#ifndef UTIL_ENCODER_H_
#define UTIL_ENCODER_H_

#include <avr/io.h>
#include <avr/pgmspace.h>
#include <stdint.h>

// #define ENCODER_FAST

// We have two update methods, a simple one (ENCODER_FAST) which can be jittery, and a more correct
// one that better respects the detents. It seems there should be a middle ground.

namespace util {

template <uint8_t A, uint8_t B>
class Encoder {
public:
  static constexpr uint8_t kMaskA = _BV(A);
  static constexpr uint8_t kMaskB = _BV(B);

  static constexpr uint8_t id() { return A; }

#ifdef ENCODER_FAST
  // For debouncing of pins, use 0x0f (b00001111) and 0x0c (b00001100) etc.
  static constexpr uint8_t kPinMask = 0x03;
  static constexpr uint8_t kPinEdge = 0x02;

  static int8_t Update(uint8_t input_state)
  {
    uint8_t a = state_a_ << 1;
    if (input_state & kMaskA) a |= 1;
    uint8_t b = state_b_ << 1;
    if (input_state & kMaskB) b |= 1;

    state_a_ = a;
    state_b_ = b;

    a &= kPinMask;
    b &= kPinMask;
    if (a == kPinEdge && b == 0x00)
      return -1;
    else if (b == kPinEdge && a == 0x00)
      return 1;
    return 0;
  }

private:
  static uint8_t state_a_;
  static uint8_t state_b_;

#else
  static int8_t Update(uint8_t input_state)
  {
    uint8_t state = (state_ << 2) & 0x0f;
    if (input_state & kMaskA) state |= 0x2;
    if (input_state & kMaskB) state |= 0x1;
    auto t = pgm_read_byte(table + state);
    state_ = state;

    int8_t inc = 0;
    if (t) {
      int8_t steps = steps_ + t;
      if (steps > 3) {
        inc = steps >> 2;
        steps -= 4;
      } else if (steps < -3) {
        inc = steps >> 2;
        steps += 4;
      }
      steps_ = steps;
    }
    return inc;
  }

private:
  static uint8_t state_;
  static int8_t steps_;

  static constexpr int8_t table[16] PROGMEM = {0, 1, -1, 0, -1, 0, 0, 1, 1, 0, 0, -1, 0, -1, 1, 0};
#endif
};

#ifdef ENCODER_FAST
template <uint8_t A, uint8_t B>
uint8_t Encoder<A, B>::state_a_ = 0xff;

template <uint8_t A, uint8_t B>
uint8_t Encoder<A, B>::state_b_ = 0xff;
#else
template <uint8_t A, uint8_t B>
uint8_t Encoder<A, B>::state_ = 0;

template <uint8_t A, uint8_t B>
int8_t Encoder<A, B>::steps_ = 0;
#endif

}  // namespace util

#endif  // UTIL_ENCODER_H_

