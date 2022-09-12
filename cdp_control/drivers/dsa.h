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
#ifndef DRIVERS_DSA_H_
#define DRIVERS_DSA_H_

#include "drivers/gpio.h"

namespace cdp {

class DSA {
public:
  using Message = uint16_t;

  static constexpr Message INVALID_MESSAGE = 0;
  static inline Message Pack(uint8_t opcode, uint8_t data)
  {
    return ((uint16_t)opcode << 8) | data;
  }

  static inline uint8_t UnpackOpcode(uint16_t message) { return message >> 8; }
  static inline uint8_t UnpackData(uint16_t message) { return message; }

  enum DSA_STATUS {
    STATUS_OK,
    STATUS_TIMEOUT,
    STATUS_ERR,
  };

  static void Init();

  static void ResetPinState();

  // Check if the other end wants to send something
  static inline bool TransmitRequested() { return !gpio::DSA_DATA::value(); }

  // If TransmitRequested was true, try and receive something
  static DSA_STATUS Receive();

  // Last message from Receive, if DSA_STATUS_OK
  static inline Message last_response() { return last_response_; }

  // Send message
  static DSA_STATUS Transmit(Message message);

  static const char *to_pstring(DSA_STATUS);

private:
  static Message last_response_;
};

}  // namespace cdp

#endif  // DRIVERS_DSA_H_

