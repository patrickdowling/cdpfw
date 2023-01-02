// cdpfw
// Copyright (C) 2023 Patrick Dowling (pld@gurkenkiste.com)
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
#include "serial_console.h"

#include <avr/pgmspace.h>
#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>

#include "cdp_control.h"
#include "drivers/serial_port.h"
#include "util/command_tokenizer.h"

// Provided via linker script
extern "C" const char *__start_cvars;
extern "C" const char *__end_cvars;
extern "C" const char *__start_ccmds;
extern "C" const char *__end_ccmds;

namespace cdp {

static char rx_buffer[64];
static util::LineBuffer<SerialConsole, 128> line_buffer;
static char fmt_buffer[128];

static void ListVariables()
{
  for (auto cvar = (const console::Variable *)&__start_cvars;
       cvar < (const console::Variable *)&__end_cvars; ++cvar) {
    SerialConsole::PrintfP(PSTR("v:%S\r\n"), cvar->name);
  }
}

static void ListCommands()
{
  for (auto ccmd = (const console::Command *)&__start_ccmds;
       ccmd < (const console::Command *)&__end_ccmds; ++ccmd) {
    SerialConsole::PrintfP(PSTR("c:%S\r\n"), ccmd->name);
  }
}

CCMD(help, []() {
  ListCommands();
  ListVariables();
});

static void DispatchCommand(const util::CommandTokenizer::Tokens tokens)
{
  if (!tokens[0]) {
    ListCommands();
    return;
  }

  SerialConsole::PrintfP(PSTR("OK\r\n>"));
}

void SerialConsole::Init()
{
  line_buffer.Reset();

  SerialPort::Init();
  SerialPort::WriteImmediateP(PSTR("Serial console\r\n>"));
  SerialPort::EnableRx();
}

void SerialConsole::Poll()
{
  auto rx_len = SerialPort::Read(rx_buffer);
  auto rx = rx_buffer;
  while (rx_len--) {
    auto c = *rx++;
    switch (c) {
      case '\n':
        DispatchCommand(util::CommandTokenizer::Tokenize(line_buffer.mutable_str()));
        line_buffer.Reset();
        break;
      default: line_buffer.Push(c);
    }
  }
}

#if 0
void SerialConsole::Printf(const char *fmt, ...)
{
  va_list args;
  va_start(args, fmt);
  vsnprintf(fmt_buffer, sizeof(fmt_buffer), fmt, args);
  va_end(args);
  SerialPort::Write(fmt_buffer);
}
#endif

void SerialConsole::PrintfP(const char *fmt, ...)
{
  va_list args;
  va_start(args, fmt);
  vsnprintf_P(fmt_buffer, sizeof(fmt_buffer), fmt, args);
  va_end(args);
  SerialPort::Write(fmt_buffer);
}

}  // namespace cdp
