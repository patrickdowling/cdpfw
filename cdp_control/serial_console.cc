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
extern "C" const console::Variable __start_cvars;
extern "C" const console::Variable __end_cvars;
extern "C" const console::Command __start_ccmds;
extern "C" const console::Command __end_ccmds;

namespace cdp {

static char rx_buffer[64];
static util::LineBuffer<SerialConsole, 128> line_buffer;
static char fmt_buffer[128];

// TODO simplify this; maybe to_string(value)?

static void PrintCvar(const console::Variable *cvar)
{
  const char *flags = cvar->readonly() ? PSTR(" RO") : PSTR("");

  switch (cvar->value.type()) {
    case console::CVAR_BOOL:
      SerialConsole::PrintfP(PSTR("%S=%S%S"), cvar->name,
                             cvar->value.read<console::CVAR_BOOL>() ? PSTR("true") : PSTR("false"),
                             flags);
      break;
    case console::CVAR_U8:
      SerialConsole::PrintfP(PSTR("%S=%02x%S"), cvar->name, cvar->value.read<console::CVAR_U8>(),
                             flags);
      break;
    case console::CVAR_STR:
      SerialConsole::PrintfP(PSTR("%S='%s'%S"), cvar->name, cvar->value.read<console::CVAR_STR>(),
                             flags);
      break;
    case console::CVAR_NONE: break;
  }
}

static bool ListVariables(const util::CommandTokenizer::Tokens &)
{
  FOREACH_CVAR (cvar) { PrintCvar(cvar); }
  return true;
}

static bool ListCommands(const util::CommandTokenizer::Tokens &)
{
  FOREACH_CCMD (ccmd) { SerialConsole::PrintfP(PSTR("%S"), ccmd->name); }
  return true;
}

static bool GetCVar(const util::CommandTokenizer::Tokens &tokens)
{
  auto name = tokens.tokens[1];
  FOREACH_CVAR (cvar) {
    if (!strcmp_P(name, cvar->name)) {
      PrintCvar(cvar);
      return true;
    }
  }
  return false;
}

CCMD(cmds, 0, ListCommands);
CCMD(vars, 0, ListVariables);
CCMD(get, 1, GetCVar);

static void DispatchCommand(const util::CommandTokenizer::Tokens &tokens)
{
  if (!tokens.num_tokens) return;

  const console::Command *cmd = nullptr;
  FOREACH_CCMD (ccmd) {
    if (!strcmp_P(tokens.tokens[0], ccmd->name)) {
      cmd = ccmd;
      break;
    }
  }

  if (!cmd || cmd->num_args() != tokens.num_tokens - 1 || !cmd->Invoke(tokens))
    SerialConsole::PrintfP(PSTR("???"));
}

void SerialConsole::Init()
{
  line_buffer.Reset();

  SerialPort::Init();
  SerialPort::WriteImmediateP(PSTR("\r\n****\r\n"));
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
  SerialPort::WriteP(PSTR("\r\n"));
}

}  // namespace cdp
