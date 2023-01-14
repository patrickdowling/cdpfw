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
#include "cover_sensor.h"

#include "util/utils.h"

namespace cdp {

uint8_t CoverSensor::threshold_ = 128 + CoverSensor::kOpenThreshold;
uint8_t CoverSensor::state_ = 0xff;
uint8_t CoverSensor::value_ = 0;

// Assumption: If the ADC isn't working, or no sensor, the value will be 0
// That should default to open.
bool CoverSensor::Init(uint8_t initial_value)
{
  value_ = initial_value;
  return initial_value > kInitThreshold;
}

void CoverSensor::set_threshold(int8_t offset)
{
  threshold_ = util::clamp(128 + offset, 0, 0xff);
}

void CoverSensor::Update(uint8_t sensor_value)
{
  sensor_value = ((value_ * 3) + sensor_value) >> 2;

  uint8_t state = state_ << 1;
  if (sensor_value > threshold_) state |= 1;

  state_ = state;
  value_ = sensor_value;
}

}  // namespace cdp
