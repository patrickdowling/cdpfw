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
#ifndef CDP_COVER_SENSOR_H
#define CDP_COVER_SENSOR_H

#include <stdint.h>

namespace cdp {

class CoverSensor {
public:
  static constexpr uint8_t kInitThreshold = 32;
  static constexpr int8_t kOpenThreshold = (150 - 128);

  static bool Init(uint8_t initial_value);

  static uint8_t threshold() { return threshold_; }
  static void set_threshold(int8_t offset);

  static void Update(uint8_t sensor_value);

  static inline bool just_opened() { return 0x80 == state_; }
  static inline bool just_closed() { return 0x7f == state_; }

  // For the debugs
  static inline uint8_t value() { return value_; }

  // Immediate check
  static inline bool is_closed() { return value_ > threshold_; }

private:
  static uint8_t threshold_;
  static uint8_t state_;
  static uint8_t value_;
};

}  // namespace cdp

#endif  // CDP_COVER_SENSOR_H
