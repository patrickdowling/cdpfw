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
#ifndef CDPRO2_H_
#define CDPRO2_H_

#include <stdint.h>

#include "avrx/progmem.h"
#include "drivers/dsa.h"
#include "util/ring_buffer.h"
#include "util/utils.h"

namespace cdp {

// Just the basic definitions for the CDPro2 DSA commands & things.
class CDPro2 {
public:
  enum Opcode : uint8_t {
    INVALID = 0x00,
    PLAY_TITLE = 0x01,
    STOP = 0x02,
    READ_TOC = 0x03,
    PAUSE = 0x04,
    PAUSE_RELEASE = 0x05,
    GET_TITLE_LENGTH = 0x09,
    GET_COMPLETE_TIME = 0x0d,
    SET_MODE = 0x15,
    SPIN_UP = 0x18,
    GET_DISC_IDENTIFIERS = 0x30,
  };

  enum Response : uint8_t {
    FOUND = 0x01,
    STOPPED = 0x02,
    DISC_STATUS = 0x03,
    ERROR_VALUES = 0x04,
    LENGTH_OF_TITLE_LSB = 0x09,
    LENGTH_OF_TITLE_MSB = 0x0a,
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

  enum ErrorCode : uint8_t {
    NO_ERROR = 0,
    NO_DISC = 2,
    SUBCODE_ERROR = 7,
    TOC_ERROR = 8,
    RADIAL_ERROR = 0x0A,
    FATAL_SLEDGE_ERROR = 0x0C,
    TURN_TABLE_MOTOR_ERROR = 0x0D,
    EMERGENCY_STOP = 0x30,
    SEARCH_TIME_OUT = 0x1F,
    SEARCH_BINARY_ERROR = 0x20,
    SEARCH_INDEX_ERROR = 0x21,
    SEARCH_TIME_ERROR = 0x22,
    ILLEGAL_COMMAND = 0x28,
    ILLEGAL_VALUE = 0x29,
    ILLEGAL_TIME_VALUE = 0x2A,
    COMMUNICATION_ERROR = 0x2B,
    RESERVED = 0x2C,
    HF_DETECTOR_ERROR = 0x2D
  };

  // A lot the codes are sequential, so it seems to make sense to just dump things in arrays.

  // TOC struct as returned via TOC_* responses
  struct TOC {
    uint8_t flags = 0;
    uint8_t data_[5] = {0, 0, 0, 0, 0};

    inline uint8_t min_track_number() const { return data_[0]; }
    inline uint8_t max_track_number() const { return data_[1]; }

    inline uint8_t disc_time_minutes() const { return data_[2]; }
    inline uint8_t disc_time_seconds() const { return data_[3]; }
    inline uint8_t disc_time_frames() const { return data_[4]; }

    inline uint8_t num_tracks() const
    {
      return max_track_number() ? 1 + max_track_number() - min_track_number() : 0;
    }

    inline bool valid() const { return 0x1f == (flags & 0x1f); }
  };

  // Currently playing title data, as returned via ACTUAL_* responses
  struct Actual {
    inline uint8_t title() const { return data_[0]; }
    inline uint8_t index() const { return data_[1]; }
    inline uint8_t minutes() const { return data_[2]; }
    inline uint8_t seconds() const { return data_[3]; }

    uint8_t data_[4] = {0, 0, 0, 0};  // NOTE Array in order of message IDs
  };

  // Try and keep track of what's going on, and what's being requested.
  // This might also be a queue, but DSA commands can override each other so that might be more
  // effort?
  struct DiscState {
    bool loaded = false;
    bool stopped = true;
    bool playing = false;
    bool paused = false;
  };

protected:
  static TOC toc_;
  static Actual actual_;
  static DiscState disc_state_;
  // static uint8_t disc_id_[5];

  static void ResetDiscState();
};

// This is what might become the template for something like a "MediaSource", but for now is just
// the nuts & bolts of controlling the CD drive.
class CDPlayer : public CDPro2 {
public:
  // Menu/Status updates
  static bool Init();
  static void Tick();
  static void GetStatus(char* buffer);

  // User player controls
  static void Play();
  static void Stop();
  static void Pause();
  static void NextTitle();
  static void PrevTitle();

  // Power handling
  static void TogglePower();
  static bool powered() { return power_state_ == POWER_ON; }

  // Our internal power states
  enum PowerState : uint8_t { POWER_OFF, POWER_UP, POWER_DOWN, POWER_ON };

private:
  static PowerState power_state_;
  static uint8_t power_sequence_;

  // Most user actions get queued in case there's a already some operation in progress
  enum ActionType : uint8_t { ACTION_PLAY, ACTION_PAUSE, ACTION_NEXT_TITLE, ACTION_PREV_TITLE };
  struct QueuedAction {
    ActionType action_type;
    uint8_t param;
  };
  static util::RingBuffer<CDPlayer, QueuedAction, 8> queued_actions_;

  // Currently in-progress message to the player
  struct AsyncCommand {
    using ResponseHandler = void (*)(Response, uint8_t);

    Opcode opcode = INVALID;
    uint8_t param = 0;
    ResponseHandler response_handler = nullptr;

    DSA::DSA_STATUS dsa_status = DSA::STATUS_ERR;
    // uint8_t retries = 0;

    inline bool valid() const { return opcode; }
  };

  static AsyncCommand async_command_;

  static uint8_t animation_;
  static char status_[40];

  static void DispatchAction(const QueuedAction& action);
  static void StartAsyncCommand(Opcode opcode, uint8_t param,
                                AsyncCommand::ResponseHandler response_handler);
  static void EndAsyncCommand();
  static void HandleResponse(DSA::Message dsa_message);

  struct PowerSequenceStep;
  static PowerState PowerSequence();

  static void ReadTOC();
  static void StopImmediate();

  static void SetFound(uint8_t param);
  static void HandleResponsePlay(Response response, uint8_t param);
  static void HandleResponsePause(Response response, uint8_t param);
  static void HandleResponseReadTOC(Response response, uint8_t param);
};

}  // namespace cdp

#endif  // CDPRO2_H_
