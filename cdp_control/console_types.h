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
#ifndef CONSOLE_TYPES_H_
#define CONSOLE_TYPES_H_

#include <avr/pgmspace.h>

#include "util/utils.h"

// The idea here is to be able to define console commands and variables whereever it's convenient,
// but so they are compiled in rather than a runtime list.
//
// This means they are placed in a dedicated section and collected by the linker.
// In addition to the PROGMEM hoops we also need some help from the linker to figure out the
// start/end of the variables. Obviously having a dedicated linker script just for this is a bit of
// luxury.
//
// Some inspiration from Quake CVars is obvious ;)
//
// Using fixed-size strings avoids having to define the progmem strings in two step.
//
// TODO Can we sort cmds/vars in linker script? This might mean expanding section to something like
// .ccvars.name

namespace console {

struct Variable {
  const char name[8];
  int value;  // testing
};

struct Command {
  using void_fn = void (*)();

  const char name[8];
  void_fn fn;

  void Invoke() const
  {
    auto f = (void_fn)pgm_read_word(&fn);
    f();
  }
};

}  // namespace console

#define CVAR(name, value)                                     \
  static constexpr console::Variable MACRO_PASTE(cvar_, name) \
      __attribute__((section(".cvars"), used)) = {#name, value}

#define CCMD(name, fn)                                       \
  static constexpr console::Command MACRO_PASTE(ccmd_, name) \
      __attribute__((section(".ccmds"), used)) = {#name, fn}

#endif  // CONSOLE_TYPES_H_
