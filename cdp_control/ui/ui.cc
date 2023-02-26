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
#include "ui.h"

#include "drivers/systick.h"
#include "remote_codes.h"
#include "timer_slots.h"
#include "util/encoder.h"

namespace ui {

// switches: 0, 1, 2, 3, 4
// encoder: A=5, B=7, SW=6

using sw1 = util::Switch<4>;
using sw2 = util::Switch<3>;
using sw3 = util::Switch<2>;
using sw4 = util::Switch<1>;
using sw5 = util::Switch<0>;
using sw_enc = util::Switch<6>;
using enc = util::Encoder<5, 7>;

/*static*/ volatile uint8_t UI::led_state_ = kLEDMask;

using namespace cdp;

void UI::Init() {}

void UI::Tick() {}

void UI::PollInputs(uint8_t input_state, uint8_t sub_tick)
{
  switch (sub_tick) {
    case 1: UpdateSwitches<sw1>(input_state); break;
    case 2: UpdateSwitches<sw2>(input_state); break;
    case 3: UpdateSwitches<sw3>(input_state); break;
    case 4: UpdateSwitches<sw4>(input_state); break;
    case 5: UpdateSwitches<sw5>(input_state); break;
    case 6: {
      UpdateSwitches<sw_enc>(input_state);
      int8_t value = enc::Update(input_state);
      if (value) EventQueue::Emplace(EVENT_ENCODER, CONTROL_ENC, value);
    }
  }
}

void UI::PollSensors(uint8_t adc_value)
{
  CoverSensor::Update(adc_value);
  if (CoverSensor::just_opened())
    EventQueue::Emplace(EVENT_SWITCH, CONTROL_COVER_SENSOR, (int8_t)1);
  else if (CoverSensor::just_closed())
    EventQueue::Emplace(EVENT_SWITCH, CONTROL_COVER_SENSOR, (int8_t)0);
}

void UI::PollIR()
{
  irmp_ISR();
  auto &event = EventQueue::Head();
  if (irmp_get_data(&event.irmp_data) && Remote::kAddress == event.irmp_data.address) {
    event.type = ui::EVENT_IR;
    event.millis = SysTick::unsafe_millis();
    EventQueue::Push();
  }
}

}  // namespace ui
