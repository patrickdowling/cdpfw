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

#include "cdp_control.h"
#include "drivers/systick.h"
#include "remote_codes.h"
#include "timer_slots.h"
#include "util/encoder.h"

namespace ui {

// switches: 0, 1, 2, 3, 4
// encoder: A=5, B=7, SW=6

using sw_PREV = util::Switch<UI::CONTROL_ID::CONTROL_SW_PREV>;
using sw_STOP = util::Switch<UI::CONTROL_ID::CONTROL_SW_STOP>;
using sw_PLAY = util::Switch<UI::CONTROL_ID::CONTROL_SW_PLAY>;
using sw_NEXT = util::Switch<UI::CONTROL_ID::CONTROL_SW_NEXT>;
using sw_MENU = util::Switch<UI::CONTROL_ID::CONTROL_SW_MENU>;
using sw_ENC = util::Switch<UI::CONTROL_ID::CONTROL_SW_ENC>;
using enc = util::Encoder<5, 7>;

/*static*/ volatile uint8_t UI::output_state_ = MCP23S17_OUTPUT_INIT;

using namespace cdp;

void UI::Init() {}

void UI::Tick() {}

void UI::PollInputs(uint8_t input_state, uint8_t sub_tick)
{
  switch (sub_tick) {
    case 1: UpdateSwitches<sw_PREV>(input_state); break;
    case 2: UpdateSwitches<sw_STOP>(input_state); break;
    case 3: UpdateSwitches<sw_PLAY>(input_state); break;
    case 4: UpdateSwitches<sw_NEXT>(input_state); break;
    case 5: UpdateSwitches<sw_MENU>(input_state); break;
    case 6: {
      UpdateSwitches<sw_ENC>(input_state);
      int8_t value = enc::Update(input_state);
      if (value) EventQueue::Emplace(EVENT_ENCODER, CONTROL_ENC, value);
    } break;
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
