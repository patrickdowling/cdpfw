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
#include "vfd.h"

#include <avr/cpufunc.h>
#include <stdarg.h>
#include <stdio.h>
#include <util/delay.h>

// TODO Timeout for waitbusy in init. Pullup is disabled, but...
// TODO Order of WaitBusy and SetupCommand/SetupData is sketchy

namespace cdp {

using namespace gpio;
using PortD = avrx::PortD::OutputRegister;

static char fmt_buffer[128];

enum VFD::Command : uint8_t {
  DISPLAY_CLEAR = 0x01,
  CURSOR_HOME = 0x02,
  DISPLAY_CONTROL = 0x08,
  SELECT_4BIT = 0x20,
  SELECT_8BIT = 0x30,
  SET_FONT = 0xf2,
  SET_GRAPHIC_CURSOR = 0xf0,
  WRITE_GRAPHIC_IMAGE = 0xf1,
};

VFD::PowerState VFD::power_state_ = VFD::POWER_OFF;
uint8_t VFD::lum_ = 0;

static inline ALWAYS_INLINE void SetupData()
{
  DISP_RS::set();
}
static inline ALWAYS_INLINE void SetupCommand()
{
  DISP_RS::reset();
}
static inline ALWAYS_INLINE void WaitBusy()
{
  // TODO Should we wait for rising edge? According to datasheet there's like 120ns after E
  // Our use case here is mostly to check for busy before setting up the next command so that
  // doesn't seem necessary.
  while (DISP_BUSY::is_high()) {}
}

// We're using D4-D7 for data, so we can in theory write a nibble at a time.
//
// However... there are other pins on the same port in use that aren't under our control:
// TXD, RXD, RC5 (MUTE is non-critical)
//
// Some of those are inputs. So it seems like doing a non-atomic read-modify-write on the port isn't
// a good idea, even if mostly it'd be disabling the pullups willy-nilly. Still not terribly
// efficient but with the limited pins, not much can be done.
//
// FWIW Using if (bit) set else reset make this function huge with a bunch of jumps.
static inline void WriteNibble(uint8_t byte)
{
  avrx::ScopedPulse<DISP_E, avrx::GPIO_SET> e;
  avrx::ResetPins<DISP_D4, DISP_D5, DISP_D6, DISP_D7>();
  if (byte & 0x10) DISP_D4::set();
  if (byte & 0x20) DISP_D5::set();
  if (byte & 0x40) DISP_D6::set();
  if (byte & 0x80) DISP_D7::set();
}

static inline void WriteByte(uint8_t byte)
{
  WaitBusy();
  WriteNibble(byte);
  // No busy wait but ca. 100ns required here, 20MHz = 50ns per instruction
  _NOP();
  _NOP();
  WriteNibble(byte << 4);
}

template <VFD::Command command, typename... Data> inline void WriteCommandData(Data &&...data)
{
  SetupCommand();
  WriteByte(command);
  SetupData();
  (WriteByte(static_cast<uint8_t>(data)), ...);
}

void VFD::Init(PowerState power_state, uint8_t lum)
{
  avrx::InitPins<DISP_D4, DISP_D5, DISP_D6, DISP_D7, DISP_RS, DISP_E, DISP_BUSY>();

  // \sa https://www.nongnu.org/avr-libc/user-manual/group__util__delay.html

  // So what we're doing here is borrowed from classic LCD displays to try and (soft) reset by
  // forcing 8-bit mode x3. Does it work? Maybe. Is it overkill? Probably. There still seem to be
  // some artifacts on reset though (hard to tell).
  _delay_ms(150);
  SetupCommand();
  WriteNibble(SELECT_8BIT);
  _delay_ms(10);
  WriteNibble(SELECT_8BIT);
  _delay_ms(0.04);
  WriteNibble(SELECT_8BIT);
  _delay_ms(0.04);
  WriteNibble(SELECT_4BIT);
  _delay_ms(0.04);
  WriteCommandData<SELECT_4BIT>(0x3);  // minimum
  _delay_ms(0.04);
  // According to datasheet:
  // "Do not read the status immediately after this command, a delay of 40us should be used
  // instead."

  Clear();
  SetLum(lum);
  SetPowerState(power_state);
}

void VFD::Clear()
{
  WriteCommandData<DISPLAY_CLEAR>();
  WriteCommandData<CURSOR_HOME>();
}

void VFD::SetPowerState(PowerState power_state)
{
  SetupCommand();
  WriteByte(DISPLAY_CONTROL | power_state);
  power_state_ = power_state;
}

void VFD::SetLum(uint8_t lum)
{
  lum = 3 - (lum & 0x3);
  if (lum != lum_) {
    WriteCommandData<SELECT_4BIT>(lum);
    // TODO Does the "wait 40us" apply here?
    lum_ = lum;
  }
}

void VFD::SetFont(Font font)
{
  WriteCommandData<SET_FONT>(font);
}

void VFD::SetFontSpacing(FontSpacing font_spacing)
{
  WriteCommandData<SET_FONT>(font_spacing);
}

void VFD::SetCursor(uint8_t line, uint8_t col)
{
  SetupCommand();
  WriteByte((line ? 0xc0 : 0x80) + col);
}

void VFD::SetGraphicCursor(uint16_t x, uint8_t y)
{
  WriteCommandData<SET_GRAPHIC_CURSOR>(x >> 8, x, y);
}

void VFD::WriteIcon16x16P(uint16_t x, uint8_t y, const uint8_t *data)
{
  SetArea(x, y, 16, 16, 'h');

  SetupData();
  uint8_t n = 16 * 16 / 8;
  while (n--) WriteByte(pgm_read_byte(data++));
}

void VFD::SetArea(uint16_t x, uint8_t y, uint16_t w, uint8_t h, char cmd)
{
  SetRect(x, y, x + w - 1, y + h - 1, cmd);
}

void VFD::SetRect(uint16_t x1, uint8_t y1, uint16_t x2, uint8_t y2, char cmd)
{
  WriteCommandData<WRITE_GRAPHIC_IMAGE>(x1 >> 8, x1, y1, x2 >> 8, x2, y2, cmd);
}

#if 0
void VFD::Print(const char *str)
{
  SetupData();
  auto c = *str++;
  while (c) {
    WriteByte(c);
    c = *str++;
  }
}

void VFD::PrintP(const char *str)
{
  SetupData();
  auto c = pgm_read_byte(str++);
  while (c) {
    WriteByte(c);
    c = pgm_read_byte(str++);
  }
}
#endif

void VFD::Printf(const char *fmt, ...)
{
  SetupData();
  va_list args;
  va_start(args, fmt);
  vsnprintf(fmt_buffer, sizeof(fmt_buffer), fmt, args);
  va_end(args);
  auto str = fmt_buffer;
  while (*str) WriteByte(*str++);
}

void VFD::PrintfP(const char *fmt, ...)
{
  SetupData();
  va_list args;
  va_start(args, fmt);
  vsnprintf_P(fmt_buffer, sizeof(fmt_buffer), fmt, args);
  va_end(args);
  auto str = fmt_buffer;
  while (*str) WriteByte(*str++);
}

}  // namespace cdp
