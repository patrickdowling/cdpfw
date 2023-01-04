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
#ifndef UTIL_COMMAND_TOKENIZER_H_
#define UTIL_COMMAND_TOKENIZER_H_

#include <stdint.h>

namespace util {

template <typename T, uint8_t N> struct LineBuffer {
  static char buffer_[N];
  static char *pos_;

  // bool overflow_

  static inline void Reset()
  {
    pos_ = buffer_;
    *pos_ = 0;
  }

  static inline void Push(char c) { *pos_++ = c; }

  static inline char *mutable_str()
  {
    *pos_ = 0;
    return buffer_;
  }
};

struct CommandTokenizer {
  struct Tokens {
    const uint8_t num_tokens;
    const char *const *tokens;
    const char *operator[](uint8_t idx) const { return tokens_[idx]; }
  };

  static constexpr int kMaxTokens = 8;
  static const char *tokens_[kMaxTokens];

  static Tokens Tokenize(char *);
};

template <typename T, uint8_t N> char LineBuffer<T, N>::buffer_[N];

template <typename T, uint8_t N> char *LineBuffer<T, N>::pos_;

}  // namespace util

#endif  // UTIL_COMMAND_TOKENIZER_H_
