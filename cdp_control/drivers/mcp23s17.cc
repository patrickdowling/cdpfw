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
#include "drivers/mcp23s17.h"

#include "cdp_control.h"

namespace cdp {

void MCP23S17::Init(uint8_t output_value)
{
  avrx::InitPins<gpio::MCP_CS>();
  SPI::Init();

  // Enables the MCP23S17 address pins
  // BANK=0:w
  // Sequential operation disabled
  WritePortRegister(PORT::A, IOCON, SEQOP);

  WritePortRegister(MCP23S17_OUTPUT_PORT, IODIR, 0x00);  // Configure as outputs
  WritePortRegister(MCP23S17_OUTPUT_PORT, GPIO, output_value);
  WritePortRegister(MCP23S17_INPUT_PORT, IODIR, 0xff);  // Configure as inputs
  WritePortRegister(MCP23S17_INPUT_PORT, GPPU, 0x00);   // external pullups
}

}  // namespace cdp
