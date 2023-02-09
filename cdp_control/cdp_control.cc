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

#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cdp_debug.h"
#include "cdpro2.h"
#include "drivers/adc.h"
#include "drivers/gpio.h"
#include "drivers/i2c.h"
#include "drivers/mcp23s17.h"
#include "drivers/relays.h"
#include "drivers/systick.h"
#include "drivers/timer.h"
#include "drivers/vfd.h"
#include "menus/menus.h"
#include "remote_codes.h"
#include "resources/resources.h"
#include "serial_console.h"
#include "settings.h"
#include "src4392.h"
#include "timer_slots.h"
#include "ui/ui.h"

// TODO There's something up with the init order. sei is enabled last, which means the serial port
// doesn't TX until then. Duh :) But enabling it earlier seems to cause some hiccups.
// TODO Some kind of timeout if no events or updates? => turn off display

namespace cdp {
GlobalState global_state = {false, VFD::kMinBrightness, {}};
DebugInfo debug_info = {0, 0};

CVAR_RO(lid_open, &global_state.lid_open);
CVAR_RW(disp_lum, &global_state.disp_brightness);
// CVAR_U8(boot, &debug_info.boot_flags);

// CVAR(src_inp, &global_state.src4392.source);
CVAR_RW(src_mute, &global_state.src4392.mute);
CVAR_RW(src_att, &global_state.src4392.attenuation);
}  // namespace cdp

using namespace cdp;
using ui::UI;

static void UpdateGlobalState()
{
  // TODO we should probabably pub/sub these values
  CoverSensor::set_threshold(Settings::get_value(SETTING_SENSOR_THRESHOLD));

  SRC4392::Update(global_state.src4392);
#ifndef DEBUG_MUTE_SYSTICK
  gpio::MUTE::set(!global_state.src4392.mute);
#endif
}

PROGMEM const char boot_msg[] = "CDPFW " CDPFW_VERSION_STRING;

static void Init()
{
  Settings::InitDefaults();
  SerialConsole::Init();

  VFD::Init(VFD::POWER_OFF, global_state.disp_brightness);
  VFD::PrintfP(boot_msg);
  VFD::SetPowerState(VFD::POWER_ON);

  gpio::MUTE::Init();
  MCP23S17::Init(MCP23S17_OUTPUT_INIT);

  Timer1::Init();  // Used by DSA + I2C
  I2C::Init();
  if (I2C::Stop()) debug_info.boot_flags |= I2C_OK;  // This doesn't actually mean much?
  if (SRC4392::Init()) debug_info.boot_flags |= SRC_OK;
  if (CDPlayer::Init()) debug_info.boot_flags |= CDP_OK;

  irmp_init();

  Adc::Init(kAdcChannel, Adc::LEFT_ALIGN);  // 8-bit
  Adc::Enable(true);
  Adc::Scan();
  while (!Adc::ready()) {}
  if (CoverSensor::Init(Adc::Read8())) { debug_info.boot_flags |= SENSOR_OK; }
#ifndef DEBUG_FORCE_LID
  global_state.lid_open = !CoverSensor::is_closed();
#else
  global_state.lid_open = false;
#endif

  SysTick::Init();
  sei();
}

static bool ProcessIRMP(const ui::Event &event)
{
#ifdef DEBUG_IRMP
  SERIAL_TRACE(PSTR("%5u IR{%02x, %04x, %04x, %02x}"), event.millis, event.irmp_data.protocol,
               event.irmp_data.address, event.irmp_data.command, event.irmp_data.flags);
#endif
  if (IRMP_FLAG_REPETITION & event.irmp_data.flags) return true;  // Ignore repeats for now
  switch (event.irmp_data.command) {
    case Remote::OFF: CDPlayer::TogglePower(); break;
    case Remote::PLAY: CDPlayer::Play(); break;
    case Remote::STOP: CDPlayer::Stop(); break;
    case Remote::MUTE: global_state.src4392.toggle_mute(); break;
    case Remote::INFO: Menus::set_current(&menu_debug); break;
    case Remote::DISP:
      global_state.disp_brightness = (global_state.disp_brightness + 1) & 0x3;
      break;
    default: return false;
  }
  return true;
}

static uint16_t last_draw_millis_ = 0;
static uint16_t last_tick_millis_ = 0;

[[noreturn]] void Run()
{
  // The general plan for the main loop is
  // - Process user input. This may change our internal state.
  // - Update SRC and other poll other hardware.
  // - Display dirty things.
  // - Clear dirty flags on internal state.

  while (true) {
    SerialConsole::Poll();

    while (UI::available()) {
      auto event = UI::PopEvent();
      if (ui::EVENT_IR == event.type) {
        if (!ProcessIRMP(event)) Menus::HandleIR(event);
      } else {
        if (event.control.id == UI::CONTROL_COVER_SENSOR) {
#ifndef DEBUG_FORCE_LID
          global_state.lid_open = event.control.value;
#else
          global_state.lid_open = false;
#endif
        } else {
          Menus::HandleEvent(event);
        }
      }
    }

    // Basically all the "user-space" times are based on SysTick::millis since nothing is time
    // critical (until it isn't
    auto millis = SysTick::millis();
    TimerSlots::Tick(millis);

    UpdateGlobalState();
    if (TimerSlots::elapsed(TIMER_SLOT_SRC_READRATIO)) {
      TimerSlots::Arm(TIMER_SLOT_SRC_READRATIO, kReadRatioTimoutMS);
      global_state.src4392.ratio = SRC4392::ReadRatio();
    }

    // NOTE TimerSlots::Tick uses absolute time, but the rest use the elapsed time.
    auto elapsed_millis = millis - last_tick_millis_;
    last_tick_millis_ = millis;

    CDPlayer::Tick(elapsed_millis);
    UI::Tick();
    Menus::Tick(elapsed_millis);

    // TODO We really ony want to redraw if something is dirty?
    if (VFD::powered()) {
      if (global_state.disp_brightness.dirty()) {
        VFD::SetLum(global_state.disp_brightness);
        global_state.disp_brightness.clear();
      }
      if (millis - last_draw_millis_ > kRedrawMs) {
        Menus::Draw();
        last_draw_millis_ = millis;
      }
    }

    // Clear all the dirties here at the end
    global_state.lid_open.clear();
    global_state.src4392.clear_dirty();

#if ENABLE_SLEEP
    // TODO Measure if this has any impact (since we get woken by ISR anyway)
    set_sleep_mode(SLEEP_MODE_IDLE);
    cli();
    {
      sleep_enable();
      sei();
      sleep_cpu();
      sleep_disable();
    }
    sei();
#endif
  }
}

__attribute__((OS_main)) int main()
{
  Init();
  // TODO Small WTF here, without a message (delay?) graphics mode on VFD fails?
  SerialConsole::PrintfP(PSTR("%S"), boot_msg);

  UI::Init();
  Menus::Init();

  global_state.src4392.force_dirty();
  UpdateGlobalState();
  TimerSlots::Arm(TIMER_SLOT_SRC_READRATIO, kReadRatioTimoutMS);

  Run();
}

SYSTICK_ISR()
{
#ifdef DEBUG_MUTE_SYSTICK
  avrx::ScopedPulse<gpio::MUTE, avrx::GPIO_SET> mute_pulse{};
#endif
  auto tick = SysTick::Tick();
  uint8_t sub_tick = tick & 0x7;

  UI::PollIR();
  // The general strategy here is to reduce time in the ISR, so the MCP updates and UI scan is
  // split into substeps. We have read input, write outputs, 5 switches, encoder/sw so that works
  // out to 16/8=2KHz-ish.
  static uint8_t input_state = 0;
  if (0 == sub_tick) {
    input_state = MCP23S17::ReadPortRegister(MCP23S17_INPUT_PORT, MCP23S17::GPIO);
  } else if (7 == sub_tick) {
    auto output_state = (~UI::led_state() & UI::kLEDMask) | Relays::output_state();
    MCP23S17::WritePortRegister(MCP23S17_OUTPUT_PORT, MCP23S17::GPIO, output_state);
  } else {
    UI::PollInputs(input_state, sub_tick);
  }
  if (Adc::ready()) {
    UI::PollSensors(Adc::Read8());
    Adc::Scan();
  }
}
