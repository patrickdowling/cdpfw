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
#ifndef UI_UI_H_
#define UI_UI_H_

#include "cover_sensor.h"
#include "ui/ui_event.h"
#include "util/ring_buffer.h"
#include "util/switch.h"

namespace ui {

class UI {
public:
  // GPA
  enum CONTROL_ID : uint8_t {
    // These are just the bit index in our input array (note reverse ordering though)
    CONTROL_SW_PREV = 0,
    CONTROL_SW_STOP = 1,
    CONTROL_SW_PLAY = 2,
    CONTROL_SW_NEXT = 3,
    CONTROL_SW_MENU = 4,
    CONTROL_SW_ENC = 6,
    CONTROL_ENC = 5,  // 7
    // This is artificial
    CONTROL_COVER_SENSOR = 8
  };

  // GPB
  enum LED_ID : uint8_t {
    LED_MUTE = _BV(0),
    LED_MENU = _BV(1),
  };
  enum OUTPUT_ID : uint8_t {
    OUT_AUX2 = _BV(2),
    OUT_AUX1 = _BV(3),
    OUT_UNUSED = _BV(4),
  };
  // 5:7-> drivers/relays.h

  static void Init();
  static void Tick();
  static void PollInputs(uint8_t input_state, uint8_t sub_tick);
  static void PollSensors(uint8_t adc_value);
  static void PollIR();

  static inline bool available() { return !EventQueue::empty(); }
  static inline Event PopEvent() { return EventQueue::Pop(); }

  // NOTE LEDs are active low, default off=high
  static inline void set_led(LED_ID pin, bool on)
  {
    if (on)
      output_state_ &= ~pin;
    else
      output_state_ |= pin;
  }

  static inline void set_output(OUTPUT_ID pin, bool on)
  {
    if (on)
      output_state_ |= pin;
    else
      output_state_ &= ~pin;
  }

  static inline uint8_t output_state() { return output_state_; }

private:
  using EventQueue = util::RingBuffer<UI, Event, 8>;

  static volatile uint8_t output_state_;

  template <typename switch_type> static inline void Update(uint8_t input_state)
  {
    switch_type::Update(input_state);
    if (switch_type::just_pressed()) {
      EventQueue::Emplace(EVENT_SWITCH, switch_type::id(), (int8_t)1);
    } else if (switch_type::just_released()) {
      EventQueue::Emplace(EVENT_SWITCH, switch_type::id(), (int8_t)0);
    }
  }

  template <typename... Sws> static inline void UpdateSwitches(uint8_t input_state)
  {
    (Update<Sws>(input_state), ...);
  }
};

}  // namespace ui

#endif  // UI_UI_H_
