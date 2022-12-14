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
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cd_player.h"
#include "drivers/adc.h"
#include "drivers/gpio.h"
#include "drivers/i2c.h"
#include "drivers/mcp23s17.h"
#include "drivers/relays.h"
#include "drivers/serial_port.h"
#include "drivers/systick.h"
#include "drivers/timer.h"
#include "drivers/vfd.h"
#include "menus.h"
#include "remote_codes.h"
#include "resources/resources.h"
#include "settings.h"
#include "src4392.h"
#include "timer_slots.h"
#include "ui/ui.h"
#include "util/command_buffer.h"

// TODO There's something up with the init order.
// sei is enabled last, which means the serial port doesn't TX until then. Duh :)
// But enabling it earlier seems to cause some hiccups.

namespace cdp {
GlobalState global_state = {false, VFD::kMinBrightness, {}, 0};
}

using namespace cdp;
using ui::UI;

static void UpdateGlobalState()
{
  // TODO we should probabably pub/sub these values
  CoverSensor::set_threshold(Settings::get_value(SETTING_SENSOR_THRESHOLD));

  SRC4392::Update(global_state.src4392);
#ifndef DEBUG_MUTE_SYSTICK
  gpio::MUTE::set(global_state.src4392.mute);
#endif
}

static inline void MuteHandler(const char *)
{
  global_state.src4392.toggle_mute();
}

static char rx_buffer[64];
static util::CommandBuffer command_buffer;

static constexpr struct {
  const char *prefix;
  const CommandHandlerFn handler_fn;
} CommandHandlers[] = {
    {"CDP", CDPlayer::CommandHandler},
    {"MUT", MuteHandler},
    {"SRC", SRC4392::CommandHandler},
#ifdef ENABLE_VFD_COMMAND_HANDLER
    {"VFD", VFD::CommandHandler},
#endif
};

static void DispatchCommand()
{
  command_buffer.Terminate();
  SerialPort::WriteP(PSTR(SERIAL_ENDL));

  bool handled = false;
  if (!command_buffer.empty()) {
    auto cmd = command_buffer.buffer();
    for (auto &handler : CommandHandlers) {
      if (!strncmp(handler.prefix, cmd, 3)) {
        cmd += 3;
        while (isspace(*cmd)) ++cmd;

        handler.handler_fn(cmd);
        handled = true;
      }
    }
  }
  if (handled) { SerialPort::WriteP(PSTR("OK" SERIAL_ENDL)); }
  command_buffer.Clear();
}

static void Init()
{
  Settings::InitDefaults();
  SerialPort::Init();
  SERIAL_BOOT_MSG(SERIAL_ENDL SERIAL_ENDL "*********" SERIAL_ENDL "CDP BOOTING..." SERIAL_ENDL);

  VFD::Init(VFD::POWER_ON, global_state.disp_brightness);
  VFD::SetArea(0, 0, 280, 16, 'C');
  VFD::SetFont(VFD::FONT_1px);
  VFD::SetFont(VFD::FONT_5x7);
  VFD::SetGraphicCursor(0, 16);
  VFD::PrintfP(PSTR("CDP BOOTING..."));

  gpio::MUTE::Init();
  global_state.boot_flags |= MUTE_OK;

  MCP23S17::Init(MCP23S17_OUTPUT_INIT);
  global_state.boot_flags |= SPI_OK;

  Timer1::Init();  // Used by DSA + I2C
  I2C::Init();
  if (I2C::Stop()) {
    SRC4392::Init();
    global_state.boot_flags |= I2C_OK;
    global_state.boot_flags |= SRC_OK;
  }
  if (CDPlayer::Init()) { global_state.boot_flags |= CDP_OK; }

  irmp_init();
  global_state.boot_flags |= IRMP_OK;

  Adc::Init(kAdcChannel, Adc::LEFT_ALIGN);  // 8-bit
  Adc::Enable(true);
  Adc::Scan();

  while (!Adc::ready()) {}
  if (CoverSensor::Init(Adc::Read8())) { global_state.boot_flags |= ADC_OK; }

  SysTick::Init();
  sei();
}

static bool ProcessIRMP(const ui::Event &event)
{
#ifdef DEBUG_IRMP
  SerialPort::PrintfP(PSTR("%5u IR{%02x, %04x, %04x, %02x}" SERIAL_ENDL), event.millis,
                      event.irmp_data.protocol, event.irmp_data.address, event.irmp_data.command,
                      event.irmp_data.flags);
#endif
  if (IRMP_FLAG_REPETITION & event.irmp_data.flags) return true;  // Ignore repeats for now
  switch (event.irmp_data.command) {
    case Remote::OFF: CDPlayer::Power(); break;
    case Remote::MUTE: global_state.src4392.toggle_mute(); break;
    case Remote::PLAY: CDPlayer::Play(); break;
    case Remote::STOP: CDPlayer::Stop(); break;
    case Remote::INFO: Menus::set_current(&menu_debug); break;
    case Remote::DISP:
      global_state.disp_brightness = (global_state.disp_brightness + 1) & 0x3;
      break;
    default: return false;
  }
  return true;
}

static void ProcessSerialPort()
{
  auto rx_len = SerialPort::Read(rx_buffer);
  auto rx = rx_buffer;
  while (rx_len--) {
    auto c = *rx++;
    switch (c) {
      case '\r': DispatchCommand(); break;
      case '\n': break;
      // case '\r': break;
      default: command_buffer.Push(c);
    }
  }
}

int main()
{
  Init();
  SerialPort::EnableRx();
  SerialPort::PrintfP(PSTR("BOOT FLAGS %02x" SERIAL_ENDL), global_state.boot_flags);
  SerialPort::WriteImmediateP(PSTR("READY" SERIAL_ENDL));

  UpdateGlobalState();
  TimerSlots::Arm(TIMER_SLOT_SRC_READRATIO, 2000);

  UI::Init();
  Menus::Init();

  // The general plan for the main loop is
  // - Process user input. This may change our internal state.
  // - Update SRC and other poll other hardware.
  // - Display dirty things.
  // - Clear dirty flags on internal state.

  while (true) {
    ProcessSerialPort();

    while (UI::available()) {
      auto event = UI::PopEvent();
      bool handled = false;
      if (ui::EVENT_IR == event.type) {
        handled = ProcessIRMP(event);
      } else {
        if (event.control.id == UI::CONTROL_COVER_SENSOR) {
          if (event.control.value) {
            CDPlayer::Stop();
            global_state.lid_open = true;
          } else {
            global_state.lid_open = false;
            CDPlayer::ReadTOC();
          }
          handled = true;
        }
        // WTF without processing the event, the VFD stops working?
        // SerialPort::PrintfP(PSTR("{%d, %02x, %d}" SERIAL_ENDL), event.type, event.control.id,
        //                     (int)event.control.value);
      }
      if (!handled) Menus::HandleEvent(event);
    }

    UpdateGlobalState();

    TimerSlots::Tick();  // Timers are based on SysTick::millis()
    UI::Tick();
    CDPlayer::Tick();
    Menus::Tick();

    if (TimerSlots::elapsed(TIMER_SLOT_SRC_READRATIO)) {
      TimerSlots::Arm(TIMER_SLOT_SRC_READRATIO, 2000);
      global_state.src4392.set_ratio(SRC4392::ReadRatio());
    }

    // TODO We really ony want to redraw if something is dirty?
    if (VFD::powered()) {
      VFD::SetLum(global_state.disp_brightness);
      Menus::Draw();
    }
    global_state.src4392.clear_dirty();
  }
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

