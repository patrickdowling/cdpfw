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

namespace cdp {

enum Source : uint8_t {
  SOURCE_I2S = 0x1,  // PORTB
};

struct SRCState {
  bool mute = true;
  uint8_t attenuation = 0xff;
  Source source = SOURCE_I2S;
  uint16_t ratio = 0xffff;
  int8_t filter = 0;

  uint8_t update_flags = 0xff;
  enum UpdateFlag : uint8_t {
    UPDATE_MUTE = (1 << 0),
    UPDATE_ATT = (1 << 1),
    UPDATE_SOURCE = (1 << 2),
    UPDATE_RATIO = (1 << 3),
    UPDATE_FILTER = (1 << 4)
  };

  inline bool dirty() const { return update_flags; }
  inline void clear_dirty() { update_flags = 0; }

  inline bool update_attenuation() const { return UPDATE_ATT & update_flags; }
  inline bool update_mute() const { return UPDATE_MUTE & update_flags; }
  inline bool update_source() const { return UPDATE_SOURCE & update_flags; }
  inline bool update_ratio() const { return UPDATE_RATIO & update_flags; }
  inline bool update_filter() const { return UPDATE_FILTER & update_flags; }

  void toggle_mute() { set_mute(!mute); }

  void set_mute(bool m)
  {
    if (m != mute) {
      mute = m;
      update_flags |= UPDATE_MUTE;
    }
  }

  void set_attenuation(uint8_t value)
  {
    if (value != attenuation) {
      attenuation = value;
      update_flags |= UPDATE_ATT;
    }
  }

  void set_source(Source src)
  {
    if (src != source) {
      source = src;
      update_flags |= UPDATE_SOURCE;
    }
  }

  void set_ratio(uint16_t value)
  {
    if (ratio != value) {
      ratio = value;
      update_flags |= UPDATE_RATIO;
    }
  }

  void set_filter(int8_t value)
  {
    if (filter != value) {
      filter = value;
      update_flags |= UPDATE_FILTER;
    }
  }
};

}  // namespace cdp

#endif /* SRC_STATE_H_ */
