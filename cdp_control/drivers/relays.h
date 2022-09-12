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
#ifndef DRIVERS_RELAYS_H_
#define DRIVERS_RELAYS_H_

#include <stdint.h>

namespace cdp {

class Relays {
public:
  enum RELAY_ID : uint8_t {
    AUX_AC = (1 << 7),
    AUX_9V = (1 << 6),
    RELAY3 = (1 << 5),
  };

  template <RELAY_ID relay_id>
  static inline void set(bool value)
  {
    if (value)
      state_ |= relay_id;
    else
      state_ &= ~relay_id;
  }

  static inline uint8_t output_state() { return state_; }

private:
  static uint8_t state_;
};

}  // namespace cdp

#endif
