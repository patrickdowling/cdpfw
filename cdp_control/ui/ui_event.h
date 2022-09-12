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
#ifndef UI_EVENT_H_
#define UI_EVENT_H_

#include <stdint.h>

#include "irmp.h"

namespace ui {

enum EventType : uint8_t {
  EVENT_NONE,
  EVENT_SWITCH,
  EVENT_ENCODER,
  EVENT_IR,
};

struct ControlEventData {
  uint8_t id = 0;
  int8_t value = 0;
};

struct Event {
  EventType type = EVENT_NONE;
  uint16_t millis = 0;

  union {
    ControlEventData control;
    IRMP_DATA irmp_data;
  };

  Event() : type{EVENT_NONE}, control{0, 0} {};
  Event(EventType t, uint8_t i, int8_t v) : type{t}, control{i, v} {}
};

}  // namespace ui

#endif  // UI_EVENT_H_

