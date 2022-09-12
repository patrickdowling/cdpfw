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
#ifndef DRIVERS_SYSTICK_H_
#define DRIVERS_SYSTICK_H_

#include <stdint.h>
#include <util/atomic.h>

namespace cdp {

class SysTick {
public:
  static void Init();

  static constexpr uint16_t kPrescaler = 8;
  static constexpr uint32_t kOverflow = (((F_CPU / F_INTERRUPTS) / kPrescaler) - 1 + 0.5);

  // If we assume F_INTERRUPTS = 16 * 1024, it's easy to generate seconds by
  // allowing the millis to be 1/1024 of a second. Albeit at the expense of slightly higher base
  // load.
  static_assert(0 == (F_INTERRUPTS % 1024));
  static_assert(16 == (F_INTERRUPTS / 1024));

  static uint16_t Tick()
  {
    ++ticks_;
    if (!(ticks_ % 16)) {
      uint16_t ms = millis_ + 1;
      if (!(ms % 1024)) ++seconds_;
      millis_ = ms;
    }
    return ticks_;
  }

  // This should still be called rarely
  static inline uint16_t millis()
  {
    uint16_t value;
    ATOMIC_BLOCK(ATOMIC_FORCEON) { value = millis_; }
    return value;
  }

  static inline uint16_t unsafe_millis() { return millis_; }

  static inline uint8_t seconds() { return seconds_; }

private:
  static uint16_t ticks_;
  static volatile uint16_t millis_;
  static volatile uint8_t seconds_;
};

}  // namespace cdp

#define SYSTICK_ISR() ISR(TIMER0_COMPA_vect)

#endif  // DRIVERS_SYSTICK_H_
