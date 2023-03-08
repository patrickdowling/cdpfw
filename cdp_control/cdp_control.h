// cdpfw
// Copyright (C) 2022, 2023 Patrick Dowling (pld@gurkenkiste.com)
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
#define MCP23S17_OUTPUT_INIT 0x03  // 0008 0011

static constexpr uint16_t kRedrawMs = 20;

static constexpr uint16_t kI2CTimeoutMs = 20;
static constexpr uint16_t kDSATimeoutMS = 250;

static constexpr uint16_t kVolumeOverlayTimeoutMS = 5000;
static constexpr uint16_t kSourceInfoTimeoutMS = 5000;

static constexpr uint16_t kReadRatioTimoutMS = 1000;

static constexpr uint8_t kAdcChannel = 7;

// The SRC sample rate is fixed since that's that what the additional DSP runs at?
static constexpr uint32_t kSrcSampleRateK = 96LU;
static constexpr uint32_t kSrcSampleRate = kSrcSampleRateK * 1000LU;

struct GlobalState {
  util::Variable<bool> lid_open{false};
  util::Variable<uint8_t> disp_brightness{0};
  SRCState src4392;
};
extern GlobalState global_state;

}  // namespace cdp

#endif  // CDP_CONTROL_H_
