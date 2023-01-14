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
#include "cdp_debug.h"
#include "cover_sensor.h"
#include "menus.h"
#include "ui.h"

namespace cdp {

using namespace ui;

class DebugMenu {
public:
  static void Init() {}
  static void Enter() { UI::set_led(UI::LED5, true); }
  static void Exit() { UI::set_led(UI::LED5, false); }
  static void Tick(uint16_t) {}
  static void HandleIR(const ui::Event &) { }
  static void HandleEvent(const ui::Event &event)
  {
    switch (event.control.id) {
      case UI::CONTROL_SW5:
        if (event.control.value) Menus::set_current(&menu_main);
        break;
    }
  }
  static void Draw()
  {
    VFD::SetCursor(0, 0);
    VFD::PrintfP(PSTR("SENS %03u"), CoverSensor::threshold());
    VFD::PrintfP(PSTR(" SRC %u"), debug_info.src_init);

    VFD::SetCursor(1, 0);
    VFD::PrintfP(PSTR("%S %03u"), global_state.lid_open ? PSTR("OPEN") : PSTR("CLOS"),
                 CoverSensor::value());
  }
};

MENU_IMPL(menu_debug, DebugMenu);

}  // namespace cdp
