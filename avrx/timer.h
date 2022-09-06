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
#ifndef AVRX_TIMER_H_
#define AVRX_TIMER_H_

#include <avr/io.h>

namespace avrx {

namespace timer {
// clang-format off
template <uint16_t pre> struct Prescaler {};
template <> struct Prescaler<1> {static constexpr uint8_t value = _BV(CS00); };
template <> struct Prescaler<8> {static constexpr uint8_t value = _BV(CS01); };
template <> struct Prescaler<64> {static constexpr uint8_t value = _BV(CS01) | _BV(CS00); };
template <> struct Prescaler<256> {static constexpr uint8_t value = _BV(CS02); };
template <> struct Prescaler<1024> {static constexpr uint8_t value = _BV(CS02) | _BV(CS00); };
// clang-format on
}  // namespace timer

class Timer0 {
public:
  template <uint16_t prescaler, uint16_t overflow>
  static void InitCTC(bool enable_isr)
  {
    static_assert(overflow > 0 && overflow <= 255);
    TCCR0A = 0x00;
    TCCR0B = 0x00;

    TCCR0B |= timer::Prescaler<prescaler>::value;
    TCCR0A |= _BV(WGM01);  // CTC mode
    OCR0A = static_cast<uint8_t>(overflow);
    TCNT0 = 0;

    if (enable_isr) {
#ifdef TIMSK0
      TIMSK0 |= _BV(OCIE0A);
#else
      TIMSK |= _BV(OCIE0A);
#endif
    }
  }
};

}  // namespace avrx

#endif  // AVRX_TIMER_H_
