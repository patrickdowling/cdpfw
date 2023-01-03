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
#ifndef CDP_CONTROL_H_
#define CDP_CONTROL_H_

#include <stdint.h>

#include "src_state.h"
#include "util/utils.h"

namespace cdp {

#define MCP23S17_INPUT_PORT MCP23S17::PORT::A
#define MCP23S17_OUTPUT_PORT MCP23S17::PORT::B
#define MCP23S17_OUTPUT_INIT 0x1f  // 0001 1111

static constexpr uint16_t kI2CTimeoutMs = 20;
static constexpr uint16_t kDSATimeoutMS = 250;

static constexpr uint16_t kVolumeOverlayTimeoutMS = 5000;
static constexpr uint16_t kSourceInfoTimeoutMS = 5000;

static constexpr uint8_t kAdcChannel = 7;

struct GlobalState {
  util::Variable<bool> lid_open;
  util::Variable<uint8_t> disp_brightness = 0;
  SRCState src4392;
};
extern GlobalState global_state;

struct DebugInfo {
  uint8_t boot_flags;
};
extern DebugInfo debug_info;

enum BOOT_FLAGS : uint8_t {
  MUTE_OK = 0x1,
  SPI_OK = 0x2,
  I2C_OK = 0x4,
  SRC_OK = 0x8,
  CDP_OK = 0x10,
  IRMP_OK = 0x20,
  ADC_OK = 0x40,
};

}  // namespace cdp

#endif  // CDP_CONTROL_H_
