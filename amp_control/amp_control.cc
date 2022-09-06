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
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <util/delay.h>

#include "avrx/avrx.h"
#include "avrx/timer.h"

// https://www.nongnu.org/avr-libc/user-manual/group__avr__sleep.html

namespace GPIO {
using SW1 = GPIO_IN(B, 0);
using IN1 = GPIO_IN(B, 1);
using IN2 = GPIO_IN(B, 2);

using OUT1 = GPIO_OUT(B, 3, GPIO_RESET);
using OUT2 = GPIO_OUT(B, 4, GPIO_RESET);
}  // namespace GPIO

// We don't have super-precise timing requirements...
//
// 8MHz / 1024 = 7812.5Hz = 0.000128s = 0.128ms
// 50Hz = 20ms / 0.128 = 156.25
// 20ms x 255 = 5.1s

static constexpr uint8_t kTickMs = 20;

#if F_CPU == 8000000
static constexpr uint16_t kPrescaler = 1024;
#elif F_CPU == 1000000
static constexpr uint16_t kPrescaler = 256;
#endif
static constexpr uint16_t kTimerOverflow = ((F_CPU / (1000 / kTickMs)) / kPrescaler - 1 + 0.5f);

int main()
{
  avrx::InitPins<GPIO::SW1, GPIO::IN1, GPIO::IN2, GPIO::OUT1, GPIO::OUT2>();
  avrx::Timer0::InitCTC<kPrescaler, kTimerOverflow>(true);
  sei();

  do {
#if 0
    cli();
    sleep_enable();
    sei();
    sleep_cpu();
    sleep_disable();
    sei();
#endif
  } while (true);
}

ISR(TIMER0_COMPA_vect)
{
  static bool b = false;

  if (b)
    GPIO::OUT1::set();
  else
    GPIO::OUT1::reset();
  b = !b;
}
