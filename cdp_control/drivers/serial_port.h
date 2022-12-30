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
#ifndef DRIVERS_SERIAL_PORT_
#define DRIVERS_SERIAL_PORT_

#include <avr/io.h>

#include "util/ring_buffer.h"
#include "util/utils.h"


namespace cdp {

class SerialPort {
public:
  // NOTE The baud rate is dependent on the the main crystal, and at 20MHz we quickly get fairly
  // inaccurate; 19.2K comes out at 0.2% error which seems feasible. OTOH 115200U seems to
  // work anyway :)

  static constexpr uint32_t kBaudRate = 115200U;

  static void Init();

  static void EnableRx();

  static inline void Rx(char c) { rx_buffer_::Push(c); }
  static void Tx();

  static inline uint8_t Read(char *buffer)
  {
    uint8_t read = 0;
    while (rx_buffer_::readable()) {
      *buffer++ = rx_buffer_::Pop();
      ++read;
    }
    return read;
  }

  static void Write(const char *buffer);
  static void WriteP(const char *buffer);

  // Write string directly to serial without buffering/ISR. Be careful not to mix & match
  static void WriteImmediateP(const char *buffer);

private:
  // These need different sizes, otherwise they clober each other.
  // That particuarly decision in avrlib did seem weird.
  using rx_buffer_ = util::RingBuffer<SerialPort, char, 64>;
  using tx_buffer_ = util::RingBuffer<SerialPort, char, 128>;
};

}  // namespace cdp

#endif  // DRIVERS_SERIAL_PORT_

