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
#include "cdpro2.h"

#include <stdio.h>
#include <string.h>

#include "avrx/macros.h"
#include "cdp_control.h"
#include "drivers/dsa.h"
#include "drivers/relays.h"
#include "serial_console.h"
#include "timer_slots.h"
#include "util/ring_buffer.h"

// TODO Hard error cases when DSA tx/rx fails. These might resolve via Stop though.
// TODO Stop while READ_TOC not complete => how to recover? Seems ok-ish already.
// TODO 0xAA in track => end of disc

// TODO in StartAsyncCommand, check for timeout and do receive if necessary? Or does the conflict
// solve itself?
// TODO Retries? \sa
// "When a (valid) command fails in execution it must be recovered by retrying the same command for
// at least two times."
//
// NOTES
// This didn't quite come together as nicely as it might and isn't (yet) super robust.
//
// The idea was for the async command handling to be simpler than trying to piece things together
// from just collecting the results in a general HandleResponse catchall. Commands in the CDPro2
// overwrite each other so perhaps getting rid of the request/response thinking might have been the
// key, and this is just overthunked. Sorry for rambling on...
//
// Although how things like sequencing (e.g. at startup or reading the disk TOC and starting
// playback) might work is a different queestion.
//
// There's also a whole chapter on "Error handling" in the DSA interface docs, that check for
// increasing timestamps etc.

namespace cdp {

/*static*/ CDPro2::TOC CDPro2::toc_ = {};
/*static*/ CDPro2::Actual CDPro2::actual_ = {};
/*static*/ CDPro2::DiscState CDPro2::disc_state_ = {};
///*static*/ uint8_t CDPro2::disc_id_[5] = {0};

/*static*/ CDPlayer::PowerState CDPlayer::power_state_ = CDPlayer::POWER_OFF;
/*static*/ uint8_t CDPlayer::power_sequence_ = 0;

/*static*/ util::RingBuffer<CDPlayer, CDPlayer::QueuedAction, 8> CDPlayer::queued_actions_;

/*static*/ CDPlayer::AsyncCommand CDPlayer::async_command_ = {};

/*static*/ uint8_t CDPlayer::animation_ = 0;
/*static*/ char CDPlayer::status_[40] = {0};

// VFD-specific chars
PROGMEM static const char busy_animation[] = {0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
                                              0x18, 0x17, 0x16, 0x15, 0x14, 0x13, 0x12, 0x11};

PROGMEM_STRINGS4(power_state_strings, "OFF", "UP", "DOWN", "ON");
const char *to_pstring(CDPlayer::PowerState ps)
{
  return (PGM_P)pgm_read_word(&(power_state_strings[ps]));
}

static util::Variable<bool> cdplayer_debug = false;
#define CDP_SERIAL_TRACE_P(...)                      \
  do {                                               \
    if (cdplayer_debug) SERIAL_TRACE_P(__VA_ARGS__); \
  } while (0)

static bool SerialCommand(const util::CommandTokenizer::Tokens &tokens)
{
  if (!strcmp_P(tokens[0], PSTR("stop"))) {
    CDPlayer::Stop();
    return true;
  } else if (!strcmp_P(tokens[0], PSTR("play"))) {
    CDPlayer::Play();
    return true;
  }

  return false;
}
CCMD(cd, 1, SerialCommand);
CVAR_RW(cd_debug, &cdplayer_debug);

void CDPro2::ResetDiscState()
{
  toc_ = {};
  actual_ = {};
  disc_state_ = {};
}

bool CDPlayer::Init()
{
  DSA::Init();

  // Power is off, so there's not much to do
  return true;
}

void CDPlayer::Tick()
{
  if (powered()) {
    // Check lid state
    if (global_state.lid_open.dirty()) {
      if (global_state.lid_open) {
        StopImmediate();
        sprintf_P(status_, PSTR("OPEN"));
      } else {
        ReadTOC();
      }
    }

    if (DSA::TransmitRequested()) {
      auto result = DSA::Receive();
      if (DSA::STATUS_OK != result.dsa_status) {
        CDP_SERIAL_TRACE_P(PSTR("RX %S"), to_pstring(result.dsa_status));
      } else {
        HandleResponse(result.message);
      }
    }

    while (!async_command_.valid() && !queued_actions_.empty()) {
      DispatchAction(queued_actions_.Pop());
    }

  } else {
    // Not powered... but we might have a powr sequence running
    if (POWER_OFF != power_state_ && TimerSlots::elapsed(TIMER_SLOT_CD_POWER)) {
      switch (PowerSequence()) {
        case POWER_OFF: break;
        case POWER_ON:
          animation_ = 0;
          ReadTOC();
          break;
        default: break;
      }
    }
  }
  auto i = animation_ + 1;
  animation_ = animation_ < sizeof(busy_animation) ? i : 0;
}

void CDPlayer::GetStatus(char *buffer)
{
  auto buf = buffer;
  if (powered()) {
    *buf++ = async_command_.valid() ? pgm_read_byte(busy_animation + animation_) : ' ';
    *buf++ = disc_state_.loaded ? 'L' : '?';
    *buf++ = disc_state_.stopped ? 'S' : '_';
    *buf++ = disc_state_.playing ? 'P' : '_';
    *buf++ = disc_state_.paused ? 'Z' : '_';
    buf += sprintf_P(buf, PSTR(" %s"), status_);
  } else {
    switch (power_state_) {
        // buf += sprintf_P(buf, to_pstring(power_state_));
      case POWER_OFF: buf += sprintf_P(buf, PSTR(" %S"), to_pstring(power_state_)); break;
      default: *buf++ = pgm_read_byte(busy_animation + animation_); break;
    }
  }
  *buf = '\0';
}

void CDPlayer::Play()
{
  if (!powered() || global_state.lid_open) return;
  queued_actions_.Emplace(ACTION_PLAY, (uint8_t)0);
}

void CDPlayer::Stop()
{
  if (!powered() || global_state.lid_open) return;

  // Stop is inelegant and just barges ahead of everything so we don't care about much except power
  // (even if those errors might be masked).
  StopImmediate();
}

void CDPlayer::Pause()
{
  if (!powered() || global_state.lid_open) return;
  queued_actions_.Emplace(ACTION_PAUSE, (uint8_t)0);
}

void CDPlayer::NextTitle()
{
  if (!powered() || global_state.lid_open) return;
  queued_actions_.Emplace(ACTION_NEXT_TITLE, (uint8_t)0);
}

void CDPlayer::PrevTitle()
{
  if (!powered() || global_state.lid_open) return;
  queued_actions_.Emplace(ACTION_PREV_TITLE, (uint8_t)0);
}

void CDPlayer::TogglePower()
{
  // This only allows changing to on/off from the off/on states.

  if (powered()) {
    CDP_SERIAL_TRACE_P(PSTR("CD: power off"));
    StopImmediate();
    power_state_ = POWER_DOWN;
    animation_ = 0;
    PowerSequence();
  } else if (POWER_OFF == power_state_ && !global_state.lid_open) {
    CDP_SERIAL_TRACE_P(PSTR("CD: power on"));
    power_state_ = POWER_UP;
    animation_ = 0;
    PowerSequence();
  }
}

void CDPlayer::DispatchAction(const QueuedAction &action)
{
  switch (action.action_type) {
    case ACTION_PLAY:
      if (disc_state_.loaded) {
        if (!disc_state_.playing)
          StartAsyncCommand(PLAY_TITLE, toc_.min_track_number(), HandleResponsePlay);
        else if (disc_state_.paused)
          StartAsyncCommand(PAUSE_RELEASE, 0, HandleResponsePause);
      } else {
        ReadTOC();
      }
      break;
      // case ACTION PAUSE:
      // If not loaded or not playing, do nothing
      // paused = !paused
    default: break;
  }
}

void CDPlayer::StartAsyncCommand(Opcode opcode, uint8_t param,
                                 AsyncCommand::ResponseHandler response_handler)
{
  auto dsa_message = DSA::Pack(opcode, param);
  auto dsa_status = DSA::Transmit(dsa_message);
  if (DSA::STATUS_OK != dsa_status) {
    sprintf_P(status_, PSTR("TX %04X %S"), dsa_message, to_pstring(dsa_status));
    CDP_SERIAL_TRACE_P(PSTR("%s"), status_);
  }

  async_command_ = {opcode, param, response_handler, dsa_status};
  animation_ = 0;
}

void CDPlayer::EndAsyncCommand()
{
  async_command_ = {};
}

void CDPlayer::ReadTOC()
{
  if (global_state.lid_open) return;

  ResetDiscState();
  StartAsyncCommand(READ_TOC, 0, HandleResponseReadTOC);
  sprintf_P(status_, PSTR("READ TOC..."));
}

void CDPlayer::StopImmediate()
{
  StartAsyncCommand(STOP, 0, nullptr);

  // Set relevant states here so we don't rely on a response
  // disc_state_.loaded unchanged
  disc_state_.stopped = true;
  disc_state_.playing = false;
  disc_state_.paused = false;
  if (disc_state_.loaded) {
    auto num_tracks = toc_.num_tracks();
    sprintf_P(status_, PSTR("%2d %S %3u:%02u"), num_tracks,
              num_tracks > 1 ? PSTR("tracks") : PSTR("track"), toc_.disc_time_minutes(),
              toc_.disc_time_seconds());
  } else {
    sprintf_P(status_, PSTR("???"));
  }
  queued_actions_.Clear();
}

void CDPlayer::SetFound(uint8_t param)
{
  switch (param) {
    case 0x41: disc_state_.paused = true; break;
    case 0x42: disc_state_.paused = false; break;

    case 0x40:  // Goto time
    case 0x43:  // spin up
    case 0x44:  // Play A-B, A-time reached
    case 0x45:  // Play A-B, B-time reached
      break;
  }
}

void CDPlayer::HandleResponsePlay(Response response, uint8_t)
{
  switch (response) {
    case ERROR_VALUES: return;

    case FOUND:
      disc_state_.stopped = false;
      disc_state_.playing = true;
      EndAsyncCommand();
      break;

    default: return;
  }
}

void CDPlayer::HandleResponsePause(Response response, uint8_t)
{
  switch (response) {
    case FOUND: EndAsyncCommand(); break;
    default: break;
  }
}

void CDPlayer::HandleResponseReadTOC(Response response, uint8_t param)
{
  switch (response) {
    case ERROR_VALUES: return;

    case TOC_MIN_TRACK_NUMBER:
    case TOC_MAX_TRACK_NUMBER:
    case TOC_TIME_MINUTES:
    case TOC_TIME_SECONDS:
    case TOC_TIME_FRAMES:
      toc_.data_[response - TOC_MIN_TRACK_NUMBER] = param;
      toc_.flags |= (0x1 << (response - TOC_MIN_TRACK_NUMBER));
      break;
    default: return;
  }

  if (toc_.valid()) {
    disc_state_.loaded = true;
    EndAsyncCommand();

    // After reading the TOC, the CD-module goes in pause mode at the beginning of the first track
    StartAsyncCommand(PLAY_TITLE, toc_.min_track_number(), HandleResponsePlay);
  }
}

void CDPlayer::HandleResponse(DSA::Message dsa_message)
{
  auto response = static_cast<Response>(DSA::UnpackOpcode(dsa_message));
  auto param = DSA::UnpackData(dsa_message);

  // This just handles some generic cases, otherwise falls through to the current command handler.
  // TODO Rules for completion of command; right now each handler needs to do that explicitly and
  // we're relying on some well-defined request/response scenarios.
  bool default_handler = true;
  switch (response) {
    case FOUND: SetFound(param); break;

    case STOPPED:
      EndAsyncCommand();
      default_handler = false;
      break;

    case ERROR_VALUES:
      if (NO_DISC == param) {
        ResetDiscState();
        sprintf_P(status_, PSTR("NO DISC"));
        default_handler = false;
      } else {
        sprintf_P(status_, PSTR("ERR %02x"), param);
      };
      EndAsyncCommand();
      break;

    case ACTUAL_TITLE:
    case ACTUAL_INDEX:
    case ACTUAL_MINUTES:
    case ACTUAL_SECONDS:
      // The response to PLAY_TITLE is ACTUAL_* + FOUND, so we just want to cache these values
      actual_.data_[response - ACTUAL_TITLE] = param;
      if (disc_state_.playing)
        sprintf_P(status_, PSTR("%3u %3u:%02u"), actual_.title(), actual_.minutes(),
                  actual_.seconds());
      default_handler = false;
      break;

    default: break;
  }

  if (default_handler && async_command_.response_handler)
    async_command_.response_handler(response, param);
}

// We're being super conservative with the timeouts here
// Datasheet mentions 150ms between 9V and 5V, 1s between on/off
//
// The mechanism relies on two things
// - That the timer won't be armed after the last step (timeout == 0)
// - The down/up state is set correctly before the first call (\sa TogglePower)
struct CDPlayer::PowerSequenceStep {
  avrx::ProgmemVariable<uint8_t> next;
  avrx::ProgmemVariable<uint16_t> timeout;
  avrx::ProgmemVariable<PowerState> state;
  avrx::ProgmemVariable<bool> aux_ac;
  avrx::ProgmemVariable<bool> aux_9v;

  DISALLOW_COPY_AND_ASSIGN(PowerSequenceStep);
};

CDPlayer::PowerState CDPlayer::PowerSequence()
{
  static PROGMEM constexpr PowerSequenceStep kPowerSequenceUp[] = {
      // start at POWER_OFF
      {1, 750, POWER_UP, true, false},
      {2, 250, POWER_UP, true, true},
      {2, 0, POWER_ON, true, true},
  };
  static PROGMEM constexpr PowerSequenceStep kPowerSequenceDown[] = {
      // start at POWER_ON
      {0, 0, POWER_OFF, false, false},
      {0, 750, POWER_DOWN, true, false},
      {1, 250, POWER_DOWN, true, true},
  };

  auto &step = POWER_UP == power_state_ ? kPowerSequenceUp[power_sequence_]
                                        : kPowerSequenceDown[power_sequence_];
  CDP_SERIAL_TRACE_P(PSTR("CD: %d { n=%d, r=%u, s=%u, AC=%d, 9V=%d }"), power_sequence_,
                     step.next.pgm_read(), step.timeout.pgm_read(), step.state.pgm_read(),
                     step.aux_ac.pgm_read(), step.aux_9v.pgm_read());

  power_sequence_ = step.next;
  TimerSlots::Arm(TIMER_SLOT_CD_POWER, step.timeout);  // 0 = disable
  power_state_ = step.state;
  Relays::set<Relays::AUX_AC>(step.aux_ac);
  Relays::set<Relays::AUX_9V>(step.aux_9v);
  return step.state;
}

}  // namespace cdp
