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
#ifndef REMOTE_CODES_H_
#define REMOTE_CODES_H_

namespace cdp {

class Remote {
public:
  static constexpr uint16_t kAddress = 0x1c;

  enum Code : uint16_t {
    OFF = 0x0c,
    NUM1 = 0x01,
    NUM2 = 0x02,
    NUM3 = 0x03,
    NUM4 = 0x04,
    NUM5 = 0x05,
    NUM6 = 0x06,
    NUM7 = 0x07,
    NUM8 = 0x08,
    NUM9 = 0x09,
    NUM0 = 0x00,

    REC = 0x75,
    INFO = 0x0f,
    DISP = 0x72,
    PP = 0x71,
    RED = REC,
    GREEN = INFO,
    YELLOW = DISP,
    BLUE = PP,

    UP = 0x20,
    DOWN = 0x21,
    LEFT = 0x11,
    RIGHT = 0x10,
    OK = 0x29,

    JUMP_BACK = 0x5c,
    PLAY = 0x32,
    JUMP_FWD = 0x22,

    IPG = 0x2d,
    TVTV = 0x72,
    EXIT = 0x74,

    SKIP_BACK = 0x37,
    STOP = 0x36,
    SLOW = 0x5d,
    SKIP_FWD = 0x34,

    LIVE = 0x7d,
    JOBS = 0x5e,
    ARCH = 0x5f,
    QUESTION = 0x73,

    MUTE = 0x0d,
    VOLUME_MINUS = 0x41,
    VOLUME_PLUS = 0x42,
    I_II = 0x48
  };
};

}  // namespace cdp

#endif  // REMOTE_CODES_H_

