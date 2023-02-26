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
#include <string.h>

#include "menus.h"
#include "settings.h"
#include "ui/ui.h"

namespace cdp {

class SettingsMenu {
public:
  static void Init();
  static void Enter();
  static void Exit();
  static void Tick(uint16_t) {}
  static void HandleIR(const ui::Event &);
  static void HandleEvent(const ui::Event &);
  static void Draw();

private:
  static int8_t current_setting_;
  static int8_t cursor_;
  static const ProgmemSettingDesc *current_setting_desc_;

  static bool dirty_;

  static void ScrollSettings(int8_t offset);
  static void MoveCursor(int8_t offset);
};

/*static*/ int8_t SettingsMenu::current_setting_ = 0;
/*static*/ int8_t SettingsMenu::cursor_ = 0;
/*static*/ bool SettingsMenu::dirty_ = false;
/*static*/ const ProgmemSettingDesc *SettingsMenu::current_setting_desc_ = nullptr;

/*static*/ void SettingsMenu::Init()
{
  current_setting_desc_ = Settings::GetDesc_P(0);
}

using namespace ui;

/*static*/ void SettingsMenu::Enter()
{
  cursor_ = -1;
  dirty_ = true;
  UI::set_led(UI::LED_MENU, true);
}

/*static*/ void SettingsMenu::Exit()
{
  UI::set_led(UI::LED_MENU, false);
}

/*static*/ void SettingsMenu::HandleIR(const ui::Event &event)
{
  (void)event;
}

/*static*/ void SettingsMenu::HandleEvent(const ui::Event &event)
{
  using ui::UI;

  if (event.control.value && event.control.id == UI::CONTROL_SW_MENU) {
    Menus::set_current(&menu_main);
    return;
  }

  if (cursor_ < 0) {
    if (event.control.value) {
      switch (event.control.id) {
        case UI::CONTROL_SW_NEXT: ScrollSettings(1); break;
        case UI::CONTROL_SW_ENC:
        case UI::CONTROL_SW_PLAY:
          cursor_ = Settings::get_value(static_cast<Setting>(current_setting_));
          dirty_ = true;
          break;
        case UI::CONTROL_SW_PREV: ScrollSettings(-1); break;
        case UI::CONTROL_ENC: ScrollSettings(event.control.value); break;
        default: break;
      }
    }
  } else {
    if (event.control.value) {
      switch (event.control.id) {
        case UI::CONTROL_SW_NEXT: MoveCursor(1); break;
        case UI::CONTROL_SW_ENC:
        case UI::CONTROL_SW_PLAY:
          Settings::apply_value(static_cast<Setting>(current_setting_), cursor_);
          dirty_ = true;
          cursor_ = -1;
          break;
        case UI::CONTROL_SW_PREV: MoveCursor(-1); break;
        case UI::CONTROL_ENC: MoveCursor(event.control.value); break;
        default: break;
      }
    }
  }
}

/*static*/ void SettingsMenu::ScrollSettings(int8_t offset)
{
  auto setting = util::clamp(current_setting_ + offset, 0, SETTING_LAST - 1);

  if (setting != current_setting_) {
    dirty_ = true;
    current_setting_ = setting;
    current_setting_desc_ = Settings::GetDesc_P(setting);
  }
}

/*static*/ void SettingsMenu::MoveCursor(int8_t offset)
{
  int8_t cursor = current_setting_desc_->clamp(cursor_ + offset);
  if (cursor != cursor_) {
    dirty_ = true;
    cursor_ = cursor;
  }
}

/*static*/ void SettingsMenu::Draw()
{
  if (dirty_) {
    VFD::Clear();
    dirty_ = false;
    VFD::SetFont(VFD::FONT_1px);
  }

  VFD::SetFont(VFD::FONT_MINI);
  VFD::SetGraphicCursor(0, 16);
  VFD::PrintfP(PSTR("%d/%d"), current_setting_ + 1, SETTING_LAST);

  // This is all somewhat fragile but Just Fits

  uint16_t x = 14;
  static constexpr uint16_t w = 8 * 6 - 4;
  static constexpr uint16_t cw = 42;

  VFD::SetFont(VFD::FONT_5x7);
  VFD::SetGraphicCursor(x + 4 + (w - strlen_P(current_setting_desc_->name) * 6 - 1) / 2, 9);
  VFD::PrintfP(current_setting_desc_->name);

  if (cursor_ < 0) VFD::SetArea(x, 0, w + 4, 11, 'O');

  x += w + 5;
  VFD::SetGraphicCursor(x, 9);
  VFD::PrintfP(PSTR("\x1D"));

  x += 6;
  auto value = Settings::get_value(static_cast<Setting>(current_setting_));
  auto strs = current_setting_desc_->value_strings();
  if (cursor_ < 0) {
    if (strs) {
      auto str = avrx::pgm_read_pointer<const char *>(&strs[value]);
      VFD::SetGraphicCursor(x + 4 + (cw - 4 - strlen_P(str) * 6 - 1) / 2, 9);
      VFD::PrintfP(str);
    } else {
      VFD::SetGraphicCursor(x + 4 + (cw - 4 - 3 * 6 - 1) / 2, 9);
      VFD::PrintfP(PSTR("%3d"), value);
    }
  } else {
    if (strs) {
      for (int8_t i = 0; i <= current_setting_desc_->max_value(); ++i) {
        auto str = avrx::pgm_read_pointer<const char *>(&strs[i]);
        VFD::SetGraphicCursor(x + 4 + (cw - 4 - strlen_P(str) * 6 - 1) / 2, 9);
        VFD::PrintfP(str);
        if (cursor_ == i) VFD::SetArea(x, 0, cw, 11, 'O');
        x += cw;
      }
    } else {
      VFD::SetGraphicCursor(x + 4 + (cw - 4 - 3 * 6 - 1) / 2, 9);
      VFD::PrintfP(PSTR("%3d"), cursor_);
      VFD::SetArea(x, 0, cw, 11, 'O');
    }
  }
}

MENU_IMPL(menu_settings, SettingsMenu);

}  // namespace cdp
