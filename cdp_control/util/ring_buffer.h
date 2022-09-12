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
#ifndef UTIL_RINGBUFFER_H_
#define UTIL_RINGBUFFER_H_

#include <stdint.h>

namespace util {

// Single producer, single consumer

template <typename Owner, typename T, uint8_t N>
class RingBuffer {
public:
  static_assert(N >= 1 && !(N & (N - 1)), "Must be power of two");

  using value_type = T;
  static constexpr uint8_t kSize = N;
  static constexpr uint8_t kMask = N - 1;

  static inline uint8_t size() { return kSize; }

  static inline void Push(T t)
  {
    uint8_t w = write_pos_;
    values_[w & kMask] = t;
    write_pos_ = w + 1;
  }

  template <typename... Args>
  static inline void Emplace(Args&&... args)
  {
    uint8_t w = write_pos_;
    values_[w & kMask] = T{args...};
    write_pos_ = w + 1;
  }

  static inline T& Head() { return values_[write_pos_ & kMask]; }
  static inline void Push() { write_pos_ = write_pos_ + 1; }

  static inline T Pop()
  {
    uint8_t r = read_pos_;
    T t = values_[r & kMask];
    read_pos_ = r + 1;
    return t;
  }

  static inline uint8_t empty() { return write_pos_ == read_pos_; }
  static inline uint8_t readable() { return write_pos_ - read_pos_; }

private:
  static value_type values_[kSize];
  static volatile uint8_t write_pos_;
  static volatile uint8_t read_pos_;
};

template <typename Owner, typename T, uint8_t N>
T RingBuffer<Owner, T, N>::values_[];
template <typename Owner, typename T, uint8_t N>
volatile uint8_t RingBuffer<Owner, T, N>::write_pos_ = 0;
template <typename Owner, typename T, uint8_t N>
volatile uint8_t RingBuffer<Owner, T, N>::read_pos_ = 0;

}  // namespace util

#endif  // UTIL_INGBUFFER_H_
