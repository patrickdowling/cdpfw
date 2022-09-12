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
#ifndef SETTINGS_H_
#define SETTINGS_H_

#include <stdint.h>

#include "avrx/avrx.h"
#include "util/utils.h"

namespace cdp {

enum Setting : uint8_t {
  SETTING_INPUT,
  SETTING_FILTER,
  SETTING_DIG_OUT,
  SETTING_SENSOR_THRESHOLD,
  SETTING_LAST,
};

// These are intended for PROGMEM use
struct ProgmemSettingDesc {
  const char name[8];

  // implicit min value is 0
  const int8_t max_value_;
  const char *const *vals_p_;

  int8_t max_value() const { return pgm_read_byte(&max_value_); }

  int8_t clamp(int8_t value) const { return util::clamp(value, (int8_t)0, max_value()); }

  const char *const *value_strings() const
  {
    return avrx::pgm_read_pointer<const char *const *>(&vals_p_);
  }
};

class Settings {
public:
  static void InitDefaults();

  static int8_t get_value(Setting setting) { return values_[setting]; }

  static const ProgmemSettingDesc *GetDesc_P(int8_t index) { return &setting_desc_[index]; }

  static bool change_value(Setting setting, int8_t delta)
  {
    return apply_value(setting, values_[setting] + delta);
  }

private:
  static int8_t values_[SETTING_LAST];

  // These are stored in PROGMEM
  static const ProgmemSettingDesc setting_desc_[SETTING_LAST];

  // Clamp and apply value; return true if changed
  static bool apply_value(Setting setting, int8_t value)
  {
    value = setting_desc_[setting].clamp(value);
    if (value != values_[setting]) {
      values_[setting] = value;
      return true;
    } else {
      return false;
    }
  }

  friend class SettingsMenu;
};

}  // namespace cdp

#endif /* SETTINGS_H_ */
