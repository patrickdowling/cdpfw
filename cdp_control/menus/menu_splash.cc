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
#include "cdp_control.h"
#include "display.h"
#include "drivers/vfd.h"
#include "menus.h"
#include "timer_slots.h"
#include "ui/ui.h"

namespace cdp {

using namespace ui;

class SplashScreen {
public:
  static void Init() {}
  static void Enter() { TimerSlots::Arm(TIMER_SLOT_MENU, 3000); }
  static void Exit() { UI::set_leds(0); }

  static void Tick()
  {
    if (w_ > 224)
      UI::set_leds(UI::LED5);
    else if (w_ > 168)
      UI::set_leds(UI::LED4);
    else if (w_ > 112)
      UI::set_leds(UI::LED3);
    else if (w_ > 56)
      UI::set_leds(UI::LED2);
    else
      UI::set_leds(UI::LED1);

    if (TimerSlots::elapsed(TIMER_SLOT_MENU)) {
      TimerSlots::Reset(TIMER_SLOT_MENU);
      Menus::set_current(&menu_main);
    }

    w_ += 4;
    if (w_ > 280) w_ = 0;
  }

  static void HandleEvent(const ui::Event &) {}

  static void Draw()
  {
    if (splash_text_.is_dirty()) {
      splash_text_.Draw();
      VFD::SetGraphicCursor(0, 16);
      VFD::PrintfP(PSTR("CDP CONTROL " CDPFW_VERSION_STRING " [0x%02x]"), global_state.boot_flags);
    }

    VFD::SetArea(0, 0, w_, 4, 'F');
  }

private:
  static GraphicText<0, 6, 280, 10, VFD::FONT_5x7, 1> splash_text_;
  static uint16_t w_;
};

uint16_t SplashScreen::w_ = 0;
GraphicText<0, 6, 280, 10, VFD::FONT_5x7, 1> SplashScreen::splash_text_;

MENU_IMPL(menu_splash, SplashScreen);

}  // namespace cdp
