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
#ifndef SRC_STATE_H_
#define SRC_STATE_H_

#include <stdint.h>
#include "util/utils.h"

namespace cdp {

enum Source : uint8_t {
  SOURCE_I2S = 0x1,  // PORTB
};

struct SRCState {

  util::Variable<bool> mute = true;
  util::Variable<uint8_t> attenuation = 0xff;

  util::Variable<Source> source = SOURCE_I2S;

  util::Variable<uint16_t> ratio = 0xffff;
  util::Variable<int8_t> filter = 0;

  inline void clear_dirty() {
    util::ClearDirtyVariables(mute, attenuation, source, ratio, filter);
  }

  void toggle_mute() { mute.set(!mute.get()); }
};

}  // namespace cdp

#endif /* SRC_STATE_H_ */
