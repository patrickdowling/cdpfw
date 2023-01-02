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
#ifndef SERIAL_CONSOLE_H_
#define SERIAL_CONSOLE_H_

#include <avr/pgmspace.h>

#include "console_types.h"

namespace cdp {

class SerialConsole {
public:
  static void Init();
  static void Poll();

  // static void Printf(const char *fmt, ...) __attribute__((format(printf, 1, 2)));
  static void PrintfP(const char *fmt, ...) __attribute__((format(printf, 1, 2)));
};

#define SERIAL_ENDL "\r\n"
#ifdef ENABLE_SERIAL_TRACE
#define SERIAL_TRACE_P(...) SerialConsole::PrintfP(__VA_ARGS__)
#else
#define SERIAL_TRACE_P(...) \
  do {                      \
  } while (0)
#endif

}  // namespace cdp

#endif  // SERIAL_CONSOLE_H_
