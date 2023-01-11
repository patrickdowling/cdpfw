// cdpfw
// Copyright (C) 2022, 2023 Patrick Dowling (pld@gurkenkiste.com)
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
#ifndef UTIL_H_
#define UTIL_H_

#define MACRO_STRING(x) #x
#define MACRO_PASTE_(x, y) x##y
#define MACRO_PASTE(x, y) MACRO_PASTE_(x, y)

#define DISALLOW_COPY_AND_ASSIGN(classname)   \
  classname(const classname &) = delete;      \
  classname(classname &&) = delete;           \
  void operator=(const classname &) = delete; \
  void operator=(classname &&) = delete

namespace util {
template <typename T>
constexpr T clamp(T value, T min, T max)
{
  if (value < min)
    return min;
  else if (value >= max)
    return max;
  else
    return value;
}

// Track a variable to know when it has been changed.
// Recommended for basic (integer) types but this isn't enforced.
//
// TODO clamp?
template <typename T>
struct Variable {
  using value_type = T;

  constexpr Variable(T value) : value_{value} {} // Not explicit for simplification

  inline T get() const { return value_; }
  inline void set(T value)
  {
    if (value_ != value) {
      value_ = value;
      dirty_ = true;
    }
  }

  // The implicit get/set operators are convenient, although they may have some unintended effects.
  operator T() const { return value_; }
  void operator=(T value) { set(value); }

  inline bool dirty() const { return dirty_; }
  inline void force_dirty() { dirty_ = true; }
  inline void clear() { dirty_ = false; }

private:
  T value_;
  bool dirty_ = false;

  Variable() = delete;
  DISALLOW_COPY_AND_ASSIGN(Variable);
};

template <typename... Ts>
void ClearDirtyVariables(Variable<Ts> &...vars)
{
  (vars.clear(), ...);
}

}  // namespace util

#endif  // UTIL_H_
