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
#include "cd_player.h"

#include <avr/pgmspace.h>
#include <stdio.h>
#include <string.h>

#include "drivers/dsa.h"
#include "drivers/relays.h"
#include "drivers/serial_port.h"
#include "timer_slots.h"

// TODO 1s timeout between power toggle

namespace cdp {

/*static*/ CDPlayer::State CDPlayer::state_ = CDPlayer::STATE_POWER_OFF;
/*static*/ DSA::DSA_STATUS CDPlayer::dsa_status_ = DSA::STATUS_OK;
/*static*/ CDPlayer::TOC CDPlayer::toc_ = {};
/*static*/ CDPlayer::PlayState CDPlayer::play_state_ = {};
/*static*/ uint8_t CDPlayer::disc_id_[5] = {0};
/*static*/ char CDPlayer::last_error_[40] = "";

bool CDPlayer::Init()
{
  DSA::Init();
  // Power is off, so there's not much to do
  return true;
}

void CDPlayer::Power()
{
  if (powered()) {
    SerialPort::PrintfP(PSTR("CD: power off" SERIAL_ENDL));
    Stop();
    Relays::set<Relays::AUX_9V>(false);
    TimerSlots::Arm(TIMER_SLOT_CD_POWER, kPowerSequenceTimeout);
    state_ = STATE_POWER_DOWN;
  } else if (STATE_POWER_OFF == state_) {
    SerialPort::PrintfP(PSTR("CD: power on" SERIAL_ENDL));
    TimerSlots::Arm(TIMER_SLOT_CD_POWER, kPowerSequenceTimeout);
    state_ = STATE_POWER_UP;
    Relays::set<Relays::AUX_AC>(true);
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
    default: SerialPort::PrintfP(PSTR("RX DSA %02x:%02x" SERIAL_ENDL), response, data);
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
        SerialPort::PrintfP(PSTR("RX %S" SERIAL_ENDL), DSA::to_pstring(dsa_status));
      } else {
        auto response = DSA::last_response();
        // SerialPort::PrintfP(PSTR("RX DSA %04x\r\n"), response);
        HandleResponse(DSA::UnpackOpcode(response), DSA::UnpackData(response));
      }
    }
  } else {
    if (STATE_POWER_OFF != state_) {
      if (TimerSlots::elapsed(TIMER_SLOT_CD_POWER)) {
        TimerSlots::Reset(TIMER_SLOT_CD_POWER);
        if (STATE_POWER_UP == state_) {
          Relays::set<Relays::AUX_9V>(true);
          state_ = STATE_STOPPED;
        } else {
          Relays::set<Relays::AUX_AC>(false);
          state_ = STATE_POWER_OFF;
        }
        SerialPort::PrintfP(PSTR("CD: %d" SERIAL_ENDL), state_);
      }
    }
  }
}

void CDPlayer::CommandHandler(const char *cmd)
{
  if (!powered()) return;

  if (!strcmp(cmd, "TOC")) {
    ReadTOC();
  } else if (!strcmp(cmd, "PLAY")) {
    Play();
  } else if (!strcmp(cmd, "STOP")) {
    Stop();
  } else {
    DSA::Message message = DSA::INVALID_MESSAGE;
    sscanf(cmd, "%x", &message);

    auto dsa_status = DSA::Transmit(message);
    SerialPort::PrintfP(PSTR("TX %04X %S" SERIAL_ENDL), message, DSA::to_pstring(dsa_status));
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
  SerialPort::PrintfP(PSTR("TX %04X %S" SERIAL_ENDL), dsa_message, DSA::to_pstring(dsa_status));
  dsa_status_ = dsa_status;
  if (DSA::STATUS_OK != dsa_status) {
    TimerSlots::Arm(TIMER_SLOT_CD_ERROR, 2000);
    sprintf_P(last_error_, PSTR("TX %04X %S"), dsa_message, DSA::to_pstring(dsa_status));
    return false;
  } else {
    return true;
  }
}

}  // namespace cdp
