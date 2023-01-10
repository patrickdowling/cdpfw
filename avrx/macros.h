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
#ifndef AVRX_MACROS_H_
#define AVRX_MACROS_H_

#define CCAT_(a, b) a##b
#define CCAT(a, b) CCAT_(a, b)

// NOTE We can remove some of this madness with parameters packs
// If twe build the array inside a struct at least

#define DEF_STRINGS2(prefix, a, b)                  \
  static const char CCAT(prefix, _0)[] PROGMEM = a; \
  static const char CCAT(prefix, _1)[] PROGMEM = b

#define DEF_STRINGS3(prefix, a, b, c) \
  DEF_STRINGS2(prefix, a, b);         \
  static const char CCAT(prefix, _2)[] PROGMEM = c

#define DEF_STRINGS4(prefix, a, b, c, d) \
  DEF_STRINGS3(prefix, a, b, c);         \
  static const char CCAT(prefix, _3)[] PROGMEM = d

#define DEF_STRINGS5(prefix, a, b, c, d, e) \
  DEF_STRINGS4(prefix, a, b, c, d);         \
  static const char CCAT(prefix, _4)[] PROGMEM = e

#define REF_STRINGS2(prefix) CCAT(prefix, _0), CCAT(prefix, _1)
#define REF_STRINGS3(prefix) REF_STRINGS2(prefix), CCAT(prefix, _2)
#define REF_STRINGS4(prefix) REF_STRINGS3(prefix), CCAT(prefix, _3)
#define REF_STRINGS5(prefix) REF_STRINGS4(prefix), CCAT(prefix, _4)

#define PROGMEM_STRINGS2(prefix, a, b) \
  DEF_STRINGS2(prefix, a, b);          \
  static const char* const PROGMEM prefix[] = {REF_STRINGS2(prefix)}

#define PROGMEM_STRINGS3(prefix, a, b, c) \
  DEF_STRINGS3(prefix, a, b, c);          \
  static const char* const PROGMEM prefix[] = {REF_STRINGS3(prefix)}

#define PROGMEM_STRINGS4(prefix, a, b, c, d) \
  DEF_STRINGS4(prefix, a, b, c, d); \
  static const char * const PROGMEM prefix[] = {REF_STRINGS4(prefix)}

#define PROGMEM_STRINGS5(prefix, a, b, c, d, e) \
  DEF_STRINGS5(prefix, a, b, c, d, e);          \
  static const char* const PROGMEM prefix[] = {REF_STRINGS5(prefix)}

#endif /* AVRX_MACROS_H_ */
