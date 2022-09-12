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
#ifndef DRIVERS_TIMER_H_
#define DRIVERS_TIMER_H_

#include <avr/io.h>
#include <stdint.h>

namespace cdp {

// The two channels share a common timebase; the idea is to use the same timer, but in different
// contexts and not both channels simultaneously.
class Timer1 {
public:

  static constexpr uint16_t kPrescaler = 1024;

  static void Init()
  {
    TCCR1A = 0;
    TCCR1B = 0;
    TCNT1 = 0;
    TCCR1B |= _BV(WGM12);             // CTC Mode
    TCCR1B |= _BV(CS12) | _BV(CS10);  // 1024
  }

  enum Channel : uint8_t { CHANNEL_A = _BV(OCF1A), CHANNEL_B = _BV(OCF1B) };

  template <Channel channel>
  struct Timeout {
    static inline void SetMs(uint16_t ms) { Timer1::SetTicks<channel>(Timer1::MsToTicks(ms)); }
    static inline void Arm()
    {
      TCNT1 = 0;
      TIFR1 |= channel;
    }
    static inline bool timeout() { return TIFR1 & channel; }
  };

  static constexpr uint16_t MsToTicks(uint16_t ms) { return ((float)F_CPU / (float)kPrescaler / 1000.f) * ms - 1; }

  template <Channel channel>
  static inline void SetTicks(uint16_t ticks);

private:
};

template <>
inline void Timer1::SetTicks<Timer1::CHANNEL_A>(uint16_t ticks)
{
  OCR1A = ticks;
}

template <>
inline void Timer1::SetTicks<Timer1::CHANNEL_B>(uint16_t ticks)
{
  OCR1B = ticks;
}

}  // namespace cdp

#endif  // DRIVERS_TIMER_H_
