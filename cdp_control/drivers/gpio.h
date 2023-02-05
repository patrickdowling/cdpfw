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
#ifndef DRIVERS_GPIO_H_
#define DRIVERS_GPIO_H_

#include "avrx/avrx.h"

namespace cdp {

namespace gpio {

// PORTB
using DISP_BUSY = GPIO_IN(B, 0);
using DISP_RS   = GPIO_OUT(B, 1, GPIO_RESET);
using DISP_E    = GPIO_OUT(B, 2, GPIO_RESET);
using SPI_MOSI  = GPIO_OUT(B, 3, GPIO_SET);
using SPI_MISO  = GPIO_IN_PU(B, 4);
using SPI_SCK   = GPIO_OUT(B, 5, GPIO_SET);

// PORTC
using MCP_CS     = GPIO_OUT(C, 0, GPIO_SET);
using DSA_DATA   = GPIO_PU(C, 1);
using DSA_ACK    = GPIO_PU(C, 2);
using DSA_STROBE = GPIO_PU(C, 3);

// We don't want to use these manually:
//  When the TWEN bit in TWCR is set (one) to enable the 2-wire serial interface, pin PC4 is
//  disconnected from the port and becomes the serial data I/O pin for the 2-wire serial interface.
//  In this mode, there is a spike filter on the pin to suppress spikes shorter than 50ns on the
//  input signal, and the pin is driven by an open drain driver with slew-rate limitation.
using SDA = GPIO_AF(C, 4);
using SCL = GPIO_AF(C, 5);

// PORTD
using RXD     = GPIO_AF(D, 0);
using TXD     = GPIO_AF(D, 1);
using RC5     = GPIO_IN_PU(D, 2); // NOTE: Initialized by IRMP
using MUTE    = GPIO_OUT(D, 3, GPIO_RESET);
using DISP_D4 = GPIO_OUT(D, 4, GPIO_RESET);
using DISP_D5 = GPIO_OUT(D, 5, GPIO_RESET);
using DISP_D6 = GPIO_OUT(D, 6, GPIO_RESET);
using DISP_D7 = GPIO_OUT(D, 7, GPIO_RESET);

}  // namespace gpio

}  // namespace cdp

#endif  // DRIVERS_GPIO_H_
