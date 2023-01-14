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
#ifndef TIMER_SLOTS_H_
#define TIMER_SLOTS_H_

#include <stdint.h>

#include "drivers/systick.h"

namespace cdp {

// These are general purpose, "user space" timers that are updated whenever there's time with the
// current SysTick::millis(). They can be used for for up to 16383 milliseconds, but aren't super
// precise.

enum TIMER_SLOT : uint8_t {
  TIMER_SLOT_SRC_READRATIO,
  TIMER_SLOT_VOL,
  TIMER_SLOT_SRC,
  TIMER_SLOT_MENU,
  TIMER_SLOT_CD_ERROR,
  TIMER_SLOT_CD_POWER,
  TIMER_SLOT_LAST,
};

class TimerSlots {
public:
  static constexpr uint8_t kNumSlots = TIMER_SLOT_LAST;

  static void Tick(uint16_t now)
  {
    for (auto &slot : slots_) { slot.Tick(now); }
    now_ = now;
  }

  // NOTE templating these has very little or no effect

  static void Arm(TIMER_SLOT slot, uint16_t timeout) { slots_[slot].Arm(now_, timeout); }
  static void Reset(TIMER_SLOT slot) { slots_[slot].Reset(); }
  static bool elapsed(TIMER_SLOT slot) { return slots_[slot].elapsed; }

private:
  struct Slot {
    uint16_t start = 0;
    uint16_t timeout = 0;
    bool elapsed = false;

    void Arm(uint16_t now, uint16_t ms)
    {
      start = now;
      timeout = ms;
      elapsed = false;
    }

    void Reset()
    {
      timeout = 0;
      elapsed = false;
    }

    void Tick(uint16_t now)
    {
      if (timeout) {
        if (now - start > timeout) {
          elapsed = true;
          timeout = 0;
        }
      }
    }
  };

  static Slot slots_[kNumSlots];
  static uint16_t now_;
};

}  // namespace cdp

#endif  // TIMER_SLOTS_H_
