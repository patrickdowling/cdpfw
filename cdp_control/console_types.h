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
#ifndef CONSOLE_TYPES_H_
#define CONSOLE_TYPES_H_

#include "avrx/progmem.h"
#include "util/command_tokenizer.h"
#include "util/utils.h"

// The idea here is to be able to define console commands and variables whereever it's convenient,
// but so they are compiled in rather than build a runtime list.
//
// Some inspiration from Quake CVars is obvious ;)
// This means vars/commands are placed in dedicated sections and collected by the linker.
// In addition to the PROGMEM hoops we also need some help from the linker to figure out the
// start/end of the variables. Obviously having a dedicated linker script just for this is a bit of
// luxury. It also makes access a bit awkward..
//
// Also there seems to be enough RAM so it's perhaps not even worth (or could be optional); the
// structs are consty anyway and that might even make it easier to make them more type specific.
//
// Using fixed-size strings avoids having to define the progmem strings in two steps (but makes
// constructors difficult).
//
// TODO Can we sort cmds/vars in linker script?
// This might mean expanding section to something like .ccvars.name
// TODO  How to handle enum types?
// TODO RO/RW versions; these might even live in their own section.

namespace console {

static constexpr int kMaxVariableNameLen = 8;
static constexpr int kMaxCommandNameLen = 8;

enum CVAR_TYPE : uint8_t { CVAR_NONE, CVAR_BOOL, CVAR_U8, CVAR_STR };

// Having a value type simplifies the constructor of Variable
struct Value {
  avrx::ProgmemVariable<CVAR_TYPE> type;
  union {
    util::Variable<bool> *const var_bool;
    util::Variable<uint8_t> *const var_u8;
    char *const str;
  };

  template <CVAR_TYPE cvar_type> auto read() const;

  constexpr Value(util::Variable<bool> *ptr) : type{CVAR_BOOL}, var_bool{ptr} {}
  constexpr Value(util::Variable<uint8_t> *ptr) : type{CVAR_U8}, var_u8{ptr} {}
  constexpr Value(char *ptr) : type{CVAR_STR}, str(ptr) {}

  DISALLOW_COPY_AND_ASSIGN(Value);
};

template <> inline auto Value::read<CVAR_BOOL>() const
{
  return reinterpret_cast<util::Variable<bool> *>(pgm_read_ptr(&var_bool))->get();
}

template <> inline auto Value::read<CVAR_U8>() const
{
  return reinterpret_cast<util::Variable<uint8_t> *>(pgm_read_ptr(&var_u8))->get();
}

template <> inline auto Value::read<CVAR_STR>() const
{
  return reinterpret_cast<const char *>(pgm_read_ptr(&str));
}

struct Variable {
  enum FLAGS : uint8_t {
    FLAG_RO = 1,
  };

  const char name[kMaxVariableNameLen + 1];

  avrx::ProgmemVariable<uint8_t> flags;
  const Value value;

  inline bool readonly() const { return pgm_read_byte(&flags) & FLAG_RO; }

  DISALLOW_COPY_AND_ASSIGN(Variable);
};

struct Command {
  using fn_type = bool (*)(const util::CommandTokenizer::Tokens&);

  const char name[kMaxCommandNameLen + 1];
  avrx::ProgmemVariable<uint8_t> num_args;
  avrx::ProgmemVariable<fn_type> fn;

  inline bool Invoke(const util::CommandTokenizer::Tokens &tokens) const
  {
    auto f = fn.pgm_read();
    return f(tokens);
  }

  DISALLOW_COPY_AND_ASSIGN(Command);
};

}  // namespace console

#define CVAR_RW(name, ptr)                                       \
  static constexpr console::Variable MACRO_PASTE(cvar_rw_, name) \
      __attribute__((section(".cvars"), used)) = {#name, 0, {ptr}}

#define CVAR_RO(name, ptr)                                       \
  static constexpr console::Variable MACRO_PASTE(cvar_ro_, name) \
      __attribute__((section(".cvars"), used)) = {#name, console::Variable::FLAG_RO, {ptr}}

#define CCMD(name, n, fn)                                       \
  static constexpr console::Command MACRO_PASTE(ccmd_, name) \
      __attribute__((section(".ccmds"), used)) = {#name, n, fn}

#define FOREACH_CVAR(x) for (auto x = &__start_cvars; x < &__end_cvars; ++x)

#define FOREACH_CCMD(x) for (auto x = &__start_ccmds; x < &__end_ccmds; ++x)

#endif  // CONSOLE_TYPES_H_
