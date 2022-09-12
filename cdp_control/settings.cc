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
#include "settings.h"

#include <avr/pgmspace.h>

#include "avrx/macros.h"
#include "cover_sensor.h"

namespace cdp {

PROGMEM_STRINGS5(setting_input_strings, "INT", "IN1", "IN2", "IN3", "IN4");
PROGMEM_STRINGS2(setting_filter_strings, "OFF", "ON");
PROGMEM_STRINGS3(setting_src_strings, "OFF", "BYPASS", "SRC");

// We keep these structs in PROGMEM
// For actual use we memcpy_P to a working copy. This saves one indirection

/*static*/ const ProgmemSettingDesc Settings::setting_desc_[SETTING_LAST] PROGMEM = {
    {"SOURCE", 4, setting_input_strings},
    {"FILTER", 1, setting_filter_strings},
    {"DIG OUT", 2, setting_src_strings},
    {"SENSOR", 127, nullptr},
};

/*static*/
int8_t Settings::values_[SETTING_LAST] = {0};

/*static*/ void Settings::InitDefaults()
{
  values_[SETTING_SENSOR_THRESHOLD] = CoverSensor::kOpenThreshold;
}

}  // namespace cdp
