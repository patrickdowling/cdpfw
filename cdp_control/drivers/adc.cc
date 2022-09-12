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
#include "drivers/adc.h"

namespace cdp {

enum ADC_REFERENCE : uint8_t { ADC_REF_EXTERN = 0x0, ADC_REF_AVCC = 0x1, ADC_REF_INTERN = 0x3 };

void Adc::Init(uint8_t channel, Alignment alignment)
{
  // internal mux channels not supported
  uint8_t admux = (ADC_REF_AVCC << REFS0) | (channel & 0x7);
  if (LEFT_ALIGN == alignment) admux |= _BV(ADLAR);
  ADMUX = admux;

  // prescaler = 7, 20MHZ / 128 = 156Khz
  // ADEN, ADSC, ADATE, ADIF
  ADCSRA = 7;
}

}  // namespace cdp
