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
#ifndef DRIVERS_VFD_H_
#define DRIVERS_VFD_H_

#include <avr/pgmspace.h>

#include "avrx/avrx.h"
#include "gpio.h"
#include "util/utils.h"

namespace cdp {

class VFD {
public:
  static constexpr uint16_t kWidth = 280;
  static constexpr uint16_t kHeight = 16;

  static constexpr uint8_t kMinBrightness = 0;
  static constexpr uint8_t kMaxBrightness = 3;

  enum PowerState : uint8_t {
    POWER_OFF = 0x00,
    POWER_ON = 0x1 << 2,
  };

  enum Font : uint8_t {
    FONT_5x7 = 'b',
    FONT_MINI = 'a',
    FONT_10x14 = 'c',
    FONT_1px = '1',
    FONT_2px = '2',
  };

  static void Init(PowerState power_state, uint8_t lum);

  static void Clear();
  static void SetPowerState(PowerState power_state);
  static void SetLum(uint8_t lum);
  static void SetFont(Font font);

  static void SetCursor(uint8_t line, uint8_t col);
#if 0
  static void PrintP(const char *pstr);
  static void Print(const char *str);
#endif
  static void Printf(const char *fmt, ...) __attribute__((format(printf, 1, 2)));
  static void PrintfP(const char *fmt, ...) __attribute__((format(printf, 1, 2)));

  static void SetGraphicCursor(uint16_t x, uint8_t y);
  static void WriteIcon16x16P(uint16_t x, uint8_t y, const uint8_t *data);
  static void SetArea(uint16_t x, uint8_t y, uint16_t w, uint8_t h, char cmd);
  static void SetRect(uint16_t x1, uint8_t y1, uint16_t x2, uint8_t y2, char cmd);

  static inline bool powered() { return power_state_; }

private:
  enum Command : uint8_t {
    DISPLAY_CLEAR = 0x01,
    CURSOR_HOME = 0x02,
    DISPLAY_CONTROL = 0x08,
    SELECT_4BIT = 0x20,
    SELECT_8BIT = 0x30,
    SET_FONT = 0xf2,
    SET_GRAPHIC_CURSOR = 0xf0,
    WRITE_GRAPHIC_IMAGE = 0xf1,
  };

  static PowerState power_state_;
  static uint8_t lum_;
};

}  // namespace cdp

#endif  // DRIVERS_VFD_H_
