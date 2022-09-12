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
#ifndef MENU_H_
#define MENU_H_

#include "ui/ui_event.h"
#include "vfd.h"

namespace cdp {

using InitFn = void (*)();
using TickFn = void (*)();
using EnterFn = void (*)();
using ExitFn = void (*)();
using EventFn = void (*)(const ui::Event &);
using DrawFn = void (*)();

struct Menu {
  const InitFn Init;
  const EnterFn Enter;
  const ExitFn Exit;
  const TickFn Tick;
  const EventFn HandleEvent;
  const DrawFn Draw;
};

class Menus {
public:
  static void Init();

  static void Tick() { current_menu->Tick(); }
  static void HandleEvent(const ui::Event &event) { current_menu->HandleEvent(event); }
  static void Draw()
  {
    if (dirty) {
      // VFD::SetArea(0, 0, 165, 16, 'C');
      VFD::Clear();
      dirty = false;
    }
    current_menu->Draw();
  }

  static void set_current(const Menu *menu)
  {
    if (current_menu) current_menu->Exit();
    menu->Enter();
    current_menu = menu;
    dirty = true;
  }

private:
  static const Menu *current_menu;
  static bool dirty;
};

extern const Menu menu_debug;
extern const Menu menu_main;
extern const Menu menu_settings;
extern const Menu menu_splash;

// TODO Put menus in PROGMEM, pgm_read_word function pointer at each call?

#define MENU_IMPL(x, cls) \
  const Menu x = {cls::Init, cls::Enter, cls::Exit, cls::Tick, cls::HandleEvent, cls::Draw}

}  // namespace cdp

#endif /* MENU_H_ */
