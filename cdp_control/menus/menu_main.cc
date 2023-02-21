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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "avrx/macros.h"
#include "cdp_control.h"
#include "cdpro2.h"
#include "display.h"
#include "drivers/vfd.h"
#include "menus.h"
#include "remote_codes.h"
#include "resources/resources.h"
#include "src4392.h"
#include "timer_slots.h"
#include "ui/ui.h"

namespace cdp {

static char status_buffer[40];

static DisplayArea<165, 0, VFD::kWidth - 165, VFD::kHeight> volume_overlay;
static GraphicText<0, 0, 165 - 16, 7, VFD::FONT_MINI, 1> source_info_text;

static const char *to_pstring(Source src)
{
  switch (src) {
    case SOURCE_I2S: return PSTR("I2S/PORTB");
    default: break;
  }
  return PSTR("????");
}

// To convert the reported input-to-output sampling ratio to a readable string, this could also just
// use a fixed table with 44.1, 48, (88?), 96, 192 and append as necessary, or use cached values for
// known ratios. For now this is just a best-guess until a different source actually gets connected
// ;) The ratio is a 5.11 fixed point value of input/ouput frequency, so our desired result is:
// f = ratio * 96Khz / 2048 (but we need to apply some rounding)
// e.g. 0x03AC = 940/2048 * 96KHz = 44.0625
static char ratio_buffer[12] = "--- Khz";
static void RatioToString(uint16_t ratio)
{
  if (ratio == 0 || ratio == 0xffff) {
    strcpy_P(ratio_buffer, PSTR("---- Khz"));
  } else {
    uint16_t f = ((uint32_t)ratio * kSrcSampleRateK * 10LU + 1024LU) >> 11;
    // f is the desired result * 10 since we want that decimal for 44.1 (which comes in as 44.0625)
    // Instead of the "expensive" divide here we could also simply manipulate the string.
    auto integral = f / 10;
    auto fractional = f - (integral * 10);
    if (fractional)
      sprintf_P(ratio_buffer, PSTR("%d.%d KHz"), integral, fractional);
    else
      sprintf_P(ratio_buffer, PSTR("%4d KHz"), integral);
  }
}

using namespace ui;

class MainMenu {
public:
  static void Init() {}
  static void Enter()
  {
    ShowSourceInfo();
    ShowVolumeOverlay();
  }
  static void Exit() {}

  static void Tick(uint16_t /*ticks*/)
  {
    if (disp_volume_overlay_ && TimerSlots::elapsed(TIMER_SLOT_VOL)) HideVolumeOverlay();
    if (disp_source_info_ && TimerSlots::elapsed(TIMER_SLOT_SRC)) HideSourceInfo();

    if (global_state.src4392.mute.dirty() || global_state.src4392.attenuation.dirty())
      ShowVolumeOverlay();

    bool show_source = global_state.src4392.source.dirty();
    if (global_state.src4392.ratio.dirty()) {
      RatioToString(global_state.src4392.ratio);
      global_state.src4392.ratio.clear();
      show_source = true;
    }
    if (show_source) ShowSourceInfo();
  }

  static void HandleIR(const ui::Event &event)
  {
    switch (event.irmp_data.command) {
      case Remote::PP: ShowSourceInfo(); break;
      default: break;
    }
  }

  static void HandleEvent(const ui::Event &event)
  {
    switch (event.control.id) {
      case UI::CONTROL_ENC: {
        int att = util::clamp(global_state.src4392.attenuation - event.control.value, 0, 255);
        global_state.src4392.attenuation = att;
      } break;
      case UI::CONTROL_SW_ENC: {
        if (event.control.value) { global_state.src4392.toggle_mute(); }
      } break;
      case UI::CONTROL_SW_MENU: {
        if (event.control.value) Menus::set_current(&menu_settings);
      } break;

      default: break;
    }
  }

  static void Draw()
  {
    CDPlayer::GetStatus(status_buffer);

    // Avoid clear + draw if string length changes. Super efficient :]
    auto p = status_buffer + strlen(status_buffer);
    while (p < status_buffer + sizeof(status_buffer) - 1) *p++ = ' ';
    // VFD::SetGraphicCursor(0, 16);
    // VFD::SetFont(VFD::FONT_5x7);
    VFD::SetCursor(1, 0);
    VFD::Printf("%.23s", status_buffer);

    if (source_info_text.is_dirty()) {
      source_info_text.Draw();
      VFD::SetGraphicCursor(0, 6);
      if (disp_source_info_) {
        VFD::PrintfP(PSTR("%S  %s"), to_pstring(global_state.src4392.source), ratio_buffer);
      } else {
        VFD::PrintfP(PSTR("%s"), ratio_buffer);
      }
    }

    if (volume_overlay.is_dirty()) {
      const bool mute = global_state.src4392.mute;
      const uint8_t db = global_state.src4392.attenuation;
      auto w =
          sprintf_P(status_buffer, PSTR(" %c%d.%ddB"), db ? '-' : ' ', (db >> 1), (db & 1) ? 5 : 0);

      volume_overlay.Draw();
      if (disp_volume_overlay_) {
        if (mute)
          VFD::WriteIcon16x16P(165, 0, resources::ICON16x16_MUTE);
        else
          VFD::SetArea(165, 0, 16, 16, 'C');

        VFD::SetFont(VFD::FONT_10x14);
        VFD::SetFont(VFD::FONT_1px);
        VFD::SetGraphicCursor(280 - w * 11, 16);
        VFD::Printf(status_buffer);
      } else {
        VFD::SetCursor(1, 40 - w);
        VFD::Printf(status_buffer);

        VFD::SetFont(VFD::FONT_MINI);
        VFD::SetFont(VFD::FONT_1px);
        VFD::SetGraphicCursor(280 - 21, 6);
        if (mute)
          VFD::PrintfP(PSTR("MUTE"));
        else
          VFD::PrintfP(PSTR("    "));
      }
    }
  }

private:
  static bool disp_volume_overlay_;
  static bool disp_source_info_;

  static void ShowVolumeOverlay()
  {
    TimerSlots::Arm(TIMER_SLOT_VOL, kVolumeOverlayTimeoutMS);
    global_state.disp_brightness = VFD::kMaxBrightness - 1;
    if (!disp_volume_overlay_) {
      disp_volume_overlay_ = true;
      volume_overlay.set_clear();
    } else {
      volume_overlay.set_dirty();
    }
  }

  static void HideVolumeOverlay()
  {
    TimerSlots::Reset(TIMER_SLOT_VOL);
    global_state.disp_brightness = VFD::kMinBrightness;
    disp_volume_overlay_ = false;
    volume_overlay.set_clear();
  }

  static void ShowSourceInfo()
  {
    TimerSlots::Arm(TIMER_SLOT_SRC, kSourceInfoTimeoutMS);
    if (!disp_source_info_) {
      disp_source_info_ = true;
      source_info_text.set_clear();
    } else {
      source_info_text.set_dirty();
    }
  }

  static void HideSourceInfo()
  {
    TimerSlots::Reset(TIMER_SLOT_SRC);
    disp_source_info_ = false;
    source_info_text.set_clear();
  }
};

bool MainMenu::disp_volume_overlay_ = false;
bool MainMenu::disp_source_info_ = false;

MENU_IMPL(menu_main, MainMenu);

}  // namespace cdp
