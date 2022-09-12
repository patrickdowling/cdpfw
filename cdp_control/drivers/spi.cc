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
#include "drivers/spi.h"

#include <avr/io.h>

#include "drivers/gpio.h"

namespace cdp {

using namespace gpio;

IOREGISTER8(SPSR);
namespace spi {
using TransferComplete = avrx::RegisterBit<SPSRRegister, SPIF>;
}

// NOTE DISP_E = SPI_SS but it doesn't seem to be needed to init things here
// In any case, if we do use it (and set it high) then we have to be careful
// so the VFD doesn't interpret it as data.

void SPI::Init()
{
  avrx::InitPins<SPI_MOSI, SPI_MISO, SPI_SCK, MCP_CS>();

  // MSB first, CPOL, CPHA defaults
  uint8_t spcr = _BV(SPE) | _BV(MSTR);

  spcr |= _BV(SPR0); // F_CPU / 16 = 1.25MHz
  SPSRRegister::Write<SPI2X>(); // x2

  SPCR = spcr;
}

void SPI::Write(const uint8_t *buffer, uint8_t buffer_length)
{
  avrx::ScopedPulse<MCP_CS, avrx::GPIO_RESET> cs;

  while (buffer_length--) {
    SPDR = *buffer++;
    while (!spi::TransferComplete::value()) {}
  }
}

uint8_t SPI::Read8(const uint8_t *buffer, uint8_t buffer_length)
{
  avrx::ScopedPulse<MCP_CS, avrx::GPIO_RESET> cs;

  while (buffer_length--) {
    SPDR = *buffer++;
    while (!spi::TransferComplete::value()) {}
  }

  return SPDR;
}

}  // namespace cdp
