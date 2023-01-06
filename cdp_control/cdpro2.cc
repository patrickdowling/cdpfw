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
#include "cdpro2.h"

#include <stdio.h>
#include <string.h>

#include "drivers/dsa.h"
#include "drivers/relays.h"
#include "serial_console.h"
#include "timer_slots.h"

// TODO Lid handling (or an Enabled flag)

namespace cdp {

/*static*/ CDPlayer::State CDPlayer::state_ = CDPlayer::STATE_POWER_OFF;
/*static*/ uint8_t CDPlayer::power_sequence_ = 0;
/*static*/ DSA::DSA_STATUS CDPlayer::dsa_status_ = DSA::STATUS_OK;
/*static*/ CDPlayer::TOC CDPlayer::toc_ = {};
/*static*/ CDPlayer::PlayState CDPlayer::play_state_ = {};
/*static*/ uint8_t CDPlayer::disc_id_[5] = {0};
/*static*/ char CDPlayer::last_error_[40] = "";

CCMD(cd_toc, 0, [](const util::CommandTokenizer::Tokens &) {
  CDPlayer::ReadTOC();
  return true;
});

// We're being super conservative with the timeouts here
// Datasheet mentions 150ms between 9V and 5V, 1s between on/off
//
// The mechanism relies on two things
// - That the timer won't be armed after the last step (timeout == 0)
// - The down/up state is set correctly before the first call (\sa TogglePower)
void CDPlayer::PowerSequence()
{
  static PROGMEM constexpr PowerSequenceStep kPowerSequenceUp[] = {
      // start at STATE_POWER_OFF
      {1, 750, STATE_POWER_UP, true, false},
      {2, 250, STATE_POWER_UP, true, true},
      {2, 0, STATE_STOPPED, true, true},
  };
  static PROGMEM constexpr PowerSequenceStep kPowerSequenceDown[] = {
      // start at STATE_STOPPED
      {0, 0, STATE_POWER_OFF, false, false},
      {0, 750, STATE_POWER_DOWN, true, false},
      {1, 250, STATE_POWER_DOWN, true, true},
  };

  auto &step = STATE_POWER_UP == state_ ? kPowerSequenceUp[power_sequence_]
                                        : kPowerSequenceDown[power_sequence_];
  SERIAL_TRACE_P(PSTR("CD: %d { n=%d, r=%u, s=%u, AC=%d, 9V=%d }"), power_sequence_,
                 step.next.pgm_read(), step.timeout.pgm_read(), step.state.pgm_read(),
                 step.aux_ac.pgm_read(), step.aux_9v.pgm_read());

  power_sequence_ = step.next;
  TimerSlots::Arm(TIMER_SLOT_CD_POWER, step.timeout);
  state_ = step.state;
  Relays::set<Relays::AUX_AC>(step.aux_ac);
  Relays::set<Relays::AUX_9V>(step.aux_9v);
}

bool CDPlayer::Init()
{
  DSA::Init();
  // Power is off, so there's not much to do
  return true;
}

void CDPlayer::TogglePower()
{
  if (powered()) {
    SERIAL_TRACE_P(PSTR("CD: power off"));
    Stop();
    state_ = STATE_POWER_DOWN;
    PowerSequence();
  } else if (STATE_POWER_OFF == state_) {
    SERIAL_TRACE_P(PSTR("CD: power on"));
    state_ = STATE_POWER_UP;
    PowerSequence();
  }
}

void CDPlayer::ReadTOC()
{
  if (!powered()) return;

  toc_ = {};
  play_state_ = {};
  if (SendCommand(READ_TOC, 0)) {
    state_ = STATE_READ_TOC;
  } else {
    state_ = STATE_STOPPED;
  }
}

void CDPlayer::Stop()
{
  if (!powered()) return;

  state_ = STATE_STOPPED;
  play_state_ = {};
  SendCommand(STOP, 0);
}

void CDPlayer::Play()
{
  if (!powered()) return;

  play_state_ = {};
  if (SendCommand(PLAY_TITLE, toc_.min_track_number)) state_ = STATE_PLAY;
}

void CDPlayer::HandleResponse(uint8_t response, uint8_t data)
{
  switch (response) {
    case ERROR_VALUES:
      toc_ = {};
      play_state_ = {};
      state_ = STATE_STOPPED;
      break;
    case STOPPED: break;
    case TOC_MIN_TRACK_NUMBER:
      toc_.min_track_number = data;
      toc_.valid |= 0x1;
      break;
    case TOC_MAX_TRACK_NUMBER:
      toc_.max_track_number = data;
      toc_.valid |= 0x2;
      break;
    case TOC_TIME_MINUTES:
      toc_.disc_time_minutes = data;
      toc_.valid |= 0x4;
      break;
    case TOC_TIME_SECONDS:
      toc_.disc_time_seconds = data;
      toc_.valid |= 0x8;
      break;
    case TOC_TIME_FRAMES:
      toc_.disc_time_frames = data;
      toc_.valid |= 0x10;
      break;

    case ACTUAL_TITLE: play_state_.title = data; break;
    case ACTUAL_INDEX: play_state_.index = data; break;
    case ACTUAL_MINUTES: play_state_.minutes = data; break;
    case ACTUAL_SECONDS: play_state_.seconds = data; break;
    default: SERIAL_TRACE_P(PSTR("RX DSA %02x:%02x"), response, data);
  }

  if (STATE_READ_TOC == state_) {
    if (toc_.is_valid()) Stop();
  }
}

void CDPlayer::Tick()
{
  if (TimerSlots::elapsed(TIMER_SLOT_CD_ERROR)) {
    TimerSlots::Reset(TIMER_SLOT_CD_ERROR);
    strcpy(last_error_, "");
  }

  if (powered()) {
    if (DSA::TransmitRequested()) {
      auto dsa_status = DSA::Receive();
      if (DSA::STATUS_OK != dsa_status) {
        SERIAL_TRACE_P(PSTR("RX %S"), to_pstring(dsa_status));
      } else {
        auto response = DSA::last_response();
        // SERIAL_TRACE_P(PSTR("RX DSA %04x\r\n"), response);
        HandleResponse(DSA::UnpackOpcode(response), DSA::UnpackData(response));
      }
    }
  } else {
    if (STATE_POWER_OFF != state_) {
      if (TimerSlots::elapsed(TIMER_SLOT_CD_POWER)) { PowerSequence(); }
    }
  }
}

void CDPlayer::Status(char *buffer)
{
  auto buf = buffer;
  // buf += sprintf_P(buf, PSTR("%d "), state_);
  if (powered()) {
    if (last_error_[0]) {
      buf += sprintf(buf, "%s", last_error_);
    } else if (STATE_PLAY == state_) {
      buf += sprintf_P(buf, PSTR("%u(%u) %3u:%02u"), play_state_.title, play_state_.index,
                       play_state_.minutes, play_state_.seconds);
    } else {
      if (toc_.is_valid()) {
        buf += sprintf_P(buf, PSTR("%2d %u:%u"), toc_.max_track_number, toc_.disc_time_minutes,
                         toc_.disc_time_seconds);
      } else if (STATE_READ_TOC == state_) {
        buf += sprintf_P(buf, PSTR("READ TOC..."));
      } else {
        buf += sprintf_P(buf, PSTR("NO DISK"));
      }
    }
  } else {
    buf += sprintf_P(buf, PSTR("OFF"));
  }
  *buf = 0;
}

bool CDPlayer::SendCommand(Command command, uint8_t data)
{
  const auto dsa_message = DSA::Pack(command, data);
  auto dsa_status = DSA::Transmit(dsa_message);
  SERIAL_TRACE_P(PSTR("TX %04X %S"), dsa_message, to_pstring(dsa_status));
  dsa_status_ = dsa_status;
  if (DSA::STATUS_OK != dsa_status) {
    TimerSlots::Arm(TIMER_SLOT_CD_ERROR, 2000);
    sprintf_P(last_error_, PSTR("TX %04X %S"), dsa_message, to_pstring(dsa_status));
    return false;
  } else {
    return true;
  }
}

}  // namespace cdp
