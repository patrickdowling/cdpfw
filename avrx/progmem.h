// cdpfw
// Copyright (C) 2023 Patrick Dowling (pld@gurkenkiste.com)
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
#ifndef AVRX_PROGMEM_H_
#define AVRX_PROGMEM_H_

#include <avr/pgmspace.h>

namespace avrx {

// TODO string wrapper?

template <typename T> struct ProgmemVariable {
  const T value_;

  // TODO This is a bit brute-force
  inline T pgm_read() const
  {
    if constexpr (1 == sizeof(T))
      return (T)(pgm_read_byte(&value_));
    else if constexpr (2 == sizeof(T))
      return (T)(pgm_read_word(&value_));
    else if constexpr (4 == sizeof(T))
      return (T)(pgm_read_dword(&value_));
    else {
      T value;
      memcpy_P(&value, &value_, sizeof(T));
      return value;
    }
  }

  inline operator T() const { return pgm_read(); }
};

// TODO This makes initialization inconvenient
template <typename T> struct ProgmemStruct {
  ProgmemStruct(const ProgmemStruct &) = delete;
  ProgmemStruct(ProgmemStruct &&) = delete;

  void operator=(const ProgmemStruct &) = delete;
  void operator=(ProgmemStruct &&) = delete;
};

}  // namespace avrx
#endif
