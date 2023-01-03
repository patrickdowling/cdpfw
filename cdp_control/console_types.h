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

#include <avr/pgmspace.h>

#include "util/command_tokenizer.h"
#include "util/utils.h"

// The idea here is to be able to define console commands and variables whereever it's convenient,
// but so they are compiled in rather than a runtime list.
//
// This means they are placed in a dedicated section and collected by the linker.
// In addition to the PROGMEM hoops we also need some help from the linker to figure out the
// start/end of the variables. Obviously having a dedicated linker script just for this is a bit of
// luxury. It also makes access a bit awkward..
//
// Some inspiration from Quake CVars is obvious ;)
//
// Using fixed-size strings avoids having to define the progmem strings in two steps.
//
// TODO Can we sort cmds/vars in linker script?
// This might mean expanding section to something like .ccvars.name
// TODO  How to handle enum types?

namespace console {

static constexpr int kMaxVariableNameLen = 8;
static constexpr int kMaxCommandNameLen = 8;

enum CVAR_TYPE : uint8_t { CVAR_NONE, CVAR_BOOL, CVAR_U8, CVAR_STR };

// Having a value type simplifies the constructor of Variable
struct Value {
  const CVAR_TYPE type_;
  union {
    util::Variable<bool> *const var_bool;
    util::Variable<uint8_t> *const var_u8;
    char *const str;
  };

  CVAR_TYPE type() const { return static_cast<console::CVAR_TYPE>(pgm_read_byte(&type_)); }

  template <CVAR_TYPE cvar_type>
  auto read() const;

  constexpr Value(util::Variable<bool> *ptr) : type_{CVAR_BOOL}, var_bool{ptr} {}
  constexpr Value(util::Variable<uint8_t> *ptr) : type_{CVAR_U8}, var_u8{ptr} {}
  constexpr Value(char *ptr) : type_{CVAR_STR}, str(ptr) {}
};

template <>
inline auto Value::read<CVAR_BOOL>() const
{
  return reinterpret_cast<util::Variable<bool> *>(pgm_read_ptr(&var_bool))->get();
}

template <>
inline auto Value::read<CVAR_U8>() const
{
  return reinterpret_cast<util::Variable<uint8_t> *>(pgm_read_ptr(&var_u8))->get();
}

template <>
inline auto Value::read<CVAR_STR>() const
{
  return reinterpret_cast<const char *>(pgm_read_ptr(&str));
}

struct Variable {
  const char name[kMaxVariableNameLen + 1];
  const Value value;
};

struct Command {
  using fn_type = void (*)(const util::CommandTokenizer::Tokens);

  const char name[kMaxCommandNameLen + 1];
  fn_type fn;

  void Invoke(const util::CommandTokenizer::Tokens tokens) const
  {
    auto f = (fn_type)pgm_read_word(&fn);
    f(tokens);
  }
};

}  // namespace console

#define CVAR(name, ptr)                                       \
  static constexpr console::Variable MACRO_PASTE(cvar_, name) \
      __attribute__((section(".cvars"), used)) = {#name, {ptr}}

#define CCMD(name, fn)                                       \
  static constexpr console::Command MACRO_PASTE(ccmd_, name) \
      __attribute__((section(".ccmds"), used)) = {#name, fn}

#define FOREACH_CVAR(x) for (auto x = &__start_cvars; x < &__end_cvars; ++x)

#define FOREACH_CCMD(x) for (auto x = &__start_ccmds; x < &__end_ccmds; ++x)

#endif  // CONSOLE_TYPES_H_
