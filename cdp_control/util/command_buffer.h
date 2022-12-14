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
#ifndef UTIL_COMMAND_BUFFER_H_
#define UTIL_COMMAND_BUFFER_H_

#include <stdint.h>

namespace util {

// TODO Making all the members actually does produce even smaller code
struct CommandBuffer {
  char buffer_[128];
  uint8_t pos_ = 0;

  inline const char *buffer() const { return buffer_; }

  inline void Push(char c) { buffer_[pos_++] = c; }

  inline bool empty() { return 0 == pos_; }

  inline void Clear() { pos_ = 0; }
  inline void Terminate() { buffer_[pos_] = 0; }
};

}  // namespace util

#endif  // UTIL_COMMAND_BUFFER_H_

