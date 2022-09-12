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
#ifndef DISPLAY_H_
#define DISPLAY_H_

#include "vfd.h"

namespace cdp {

template <uint16_t x, uint8_t y, uint16_t w, uint8_t h>
class DisplayArea {
public:
  void Draw()
  {
    if (clear_) VFD::SetArea(x, y, w, h, 'C');
    clear_ = dirty_ = false;
  }

  bool is_dirty() const { return dirty_; }
  void set_dirty() { dirty_ = true; }
  void set_clear() { clear_ = dirty_ = true; }

protected:
  bool clear_ = true;
  bool dirty_ = true;
};

template <uint16_t x, uint8_t y, uint16_t w, uint8_t h, VFD::Font font, int font_spacing>
class GraphicText : public DisplayArea<x, y, w, h> {
public:

  static_assert(1 == font_spacing || 2 == font_spacing);

  void Draw()
  {
    DisplayArea<x, y, w, h>::Draw();
    VFD::SetFont(font);
    if (1 == font_spacing) VFD::SetFont(VFD::FONT_1px);
    if (2 == font_spacing) VFD::SetFont(VFD::FONT_2px);
  }
};

}  // namespace cdp

#endif /* DISPLAY_H_ */
