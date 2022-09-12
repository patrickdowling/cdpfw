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
#ifndef CD_PLAYER_H_
#define CD_PLAYER_H_

#include <stdint.h>

#include "dsa.h"

namespace cdp {

class CDPlayer {
public:
  static constexpr uint16_t kPowerSequenceTimeout = 150;

  enum Command : uint8_t {
    PLAY_TITLE = 0x01,
    STOP = 0x02,
    READ_TOC = 0x03,
    PAUSE = 0x04,
    PAUSE_RELEASE = 0x05,

    GET_TITLE_LENGTH = 0x09,
    GET_COMPLETE_TIME = 0x0d,

    SET_MODE = 0x15,

    GET_DISC_IDENTIFIERS = 0x30,
  };

  enum Response : uint8_t {
    FOUND = 0x01,
    STOPPED = 0x02,
    DISC_STATUS = 0x03,
    ERROR_VALUES = 0x04,
    LENGTH_OF_TITLE_LSB = 0x09,
    LENGTH_OF_TITLE_MBS = 0x0a,
    ACTUAL_TITLE = 0x10,
    ACTUAL_INDEX = 0x11,
    ACTUAL_MINUTES = 0x12,
    ACTUAL_SECONDS = 0x13,
    ABSOLUTE_TIME_MINUTES = 0x14,
    ABSOLUTE_TIME_SECONDS = 0x15,
    ABSOLUTE_TIME_FRAMES = 0x16,
    MODE_STATUS = 0x17,
    TOC_MIN_TRACK_NUMBER = 0x20,
    TOC_MAX_TRACK_NUMBER = 0x21,
    TOC_TIME_MINUTES = 0x22,
    TOC_TIME_SECONDS = 0x23,
    TOC_TIME_FRAMES = 0x24,
    DISC_IDENTIFIER_0 = 0x30,
    DISC_IDENTIFIER_1 = 0x31,
    DISC_IDENTIFIER_2 = 0x32,
    DISC_IDENTIFIER_3 = 0x33,
    DISC_IDENTIFIER_4 = 0x34,
    TOC_CLEARED = 0x6a,
    DAC_MODE = 0x70,
    SERVO_VERSION_NUMBER = 0xf0,
  };

  static bool Init();
  static void Tick();
  static void CommandHandler(const char *cmd);

  static void Status(char *buffer);

  static void Power();

  static void ReadTOC();
  static void Play();
  static void Stop();

  static bool powered() { return state_ >= STATE_STOPPED; }

private:
  enum State : uint8_t {
    STATE_POWER_OFF,
    STATE_POWER_UP,
    STATE_POWER_DOWN,
    STATE_STOPPED,
    STATE_READ_TOC,
    STATE_PLAY
  };

  struct TOC {
    uint8_t valid = 0;
    uint8_t min_track_number = 0;
    uint8_t max_track_number = 0;
    uint8_t disc_time_minutes = 0;
    uint8_t disc_time_seconds = 0;
    uint8_t disc_time_frames = 0;
    bool is_valid() const { return 0x1f == valid; }
  };

  struct PlayState {
    uint8_t title = 0;
    uint8_t index = 0;
    uint8_t minutes = 0;
    uint8_t seconds = 0;
  };

  static State state_;
  static DSA::DSA_STATUS dsa_status_;

  static TOC toc_;
  static PlayState play_state_;
  static uint8_t disc_id_[5];
  static char last_error_[40];

  static bool SendCommand(Command command, uint8_t data);
  static void HandleResponse(uint8_t response, uint8_t data);
};

}  // namespace cdp

#endif  // CD_PLAYER_H_
