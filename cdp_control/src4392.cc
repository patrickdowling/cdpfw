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
#include "src4392.h"

#include <avr/pgmspace.h>
#include <string.h>

#include "cdp_debug.h"
#include "serial_console.h"
#include "src_state.h"

namespace cdp {
#define SRC_REGISTER(...) RegisterData::Make<__VA_ARGS__>()

// TODO Explicit bit fields/descriptions

// NOTE
//
// MCLK should be shared with PCM1794A i.e. 96KHz?

// SRC => PORTA => DSP => DAC
// PORTA
// 0x39 = [AOUTS = b11 = SRC][AM/S = master, AFMT = b001 = 24-Bit Philips I2S]
// 0x01 = [0][ACLK = 0b00 = MCLK, ADIV = b01 = Divide by 256
//
// I2S => PORTB => SRC
// PORTB
// 0x01 = [BOUTS = 0b00 = Port B Input][AMS/s = slave, AFMT = 0b001 = 24-Bit Philips I2S]
// 0x01 = [0][ADIV = b01 = Divide by 256

bool SRC4392::Init()
{
  // The init sequence now lives in PROGMEM. This provides not much benefit.
  // Initially it looked like the a templated sequence (using I2C::WriteBytes) was generated "a lot"
  // of code. After re-writing things to use PROGMEM and a static sequence, the size is effectively
  // the same though. It may also be slightly slower per byte (since pgm_read_byte) but still ends
  // up looking a bit simpler, and that's not a critical path, so it's left as-is. Remember kids,
  // always check your assumptions.
  static PROGMEM constexpr RegisterData init_sequence[] = {
      SRC_REGISTER(PAGE_SELECTION, 0x00),
      SRC_REGISTER(RESET, 0x3f),  // Disable all PD*, i.e. enable all
      SRC_REGISTER(PORTA_CONTROL, 0x39, 0x01, /*PORTB_CONTROL=>*/ 0x01, 0x01),
      SRC_REGISTER(TRANSMITTER_CONTROL, 0x38, 0x00),
      SRC_REGISTER(RECEIVER_CONTROL, 0x08, 0x19, 0x22),
      SRC_REGISTER(SRC_CONTROL, SOURCE_I2S, 0x00),
      SRC_REGISTER(SRC_CONTROL_ATT_L, 0xff, 0xff),
      SRC_REGISTER(GPO2, 0x01),  // GPO2 = TAS3103 !RST
      SRC_REGISTER(GPO1, 0x00),  // GPO1 = PCM1794:3 DEMphasis
      SRC_REGISTER(TRANSMITTER_CONTROL, 0x38, 0x07),
  };
  static constexpr uint8_t num_registers = sizeof(init_sequence) / sizeof(RegisterData);

  uint8_t success = 0;
  for (const auto &register_data : init_sequence) {
    if (!WriteP(register_data)) break;
    ++success;
  }

  SERIAL_TRACE_P(PSTR("SRC4392::Init %d\r\n"), success);
  debug_info.src_init = success;
  return success == num_registers;
}

void SRC4392::Update(const SRCState &state)
{
  // In this case, using RegisterData structs increases code size (by like 300 bytes)
  if (Write<PAGE_SELECTION>(0x00)) {
    if (state.source.dirty()) Write<SRC_CONTROL>(state.source | (state.mute ? SRC_MUTE : 0));
    if (state.attenuation.dirty()) Write<SRC_CONTROL_ATT_L>(state.attenuation, state.attenuation);
  }
}

// TODO Correct shifting
// \sa menu_main.cc:RatioToString
// 0x32 SRI[4:0] Integer Part of the Input-to-Output Sampling Ratio
// 0x33 SRF[10:0] Fractional Part of the Input-to-Output Sampling Ratio
// In order to properly read back the ratio, these registers must be read back in sequence, starting
// with register 0x32.
uint16_t SRC4392::ReadRatio()
{
  static RegisterData ratio_readback = SRC_REGISTER(SRC_RATIO_READBACK_SRI, 0, 0);
  if (Read(ratio_readback))
    return ((uint16_t)ratio_readback.data[0] << 8) | ((uint16_t)ratio_readback.data[1]);
  else
    return 0xffff;
}

}  // namespace cdp
