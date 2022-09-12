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
#ifndef DRIVERS_ADC_H_
#define DRIVERS_ADC_H_

#include <stdint.h>

#include "avrx/avrx.h"

namespace cdp {

IOREGISTER8(ADCSRA);

namespace adc {
using Enable = avrx::RegisterBit<ADCSRARegister, ADEN>;
using Convert = avrx::RegisterBit<ADCSRARegister, ADSC>;
}  // namespace adc

class Adc {
public:
  enum Alignment { LEFT_ALIGN, RIGHT_ALIGN };

  static void Init(uint8_t channel, Alignment alignment);
  static void Enable(bool enable)
  {
    if (enable)
      adc::Enable::set();
    else
      adc::Enable::reset();
  }

  static inline void Scan() { adc::Convert::set(); }
  static inline void Wait()
  {
    while (adc::Convert::value()) {}
  }

  static inline bool ready() { return !adc::Convert::value(); }

  static inline uint16_t Read16()
  {
    uint16_t l = ADCL;
    return (ADCH << 8) | l;
  }

  static inline uint8_t Read8() { return ADCH; }
};

}  // namespace cdp

#endif  // DRIVERS_ADC_H_
