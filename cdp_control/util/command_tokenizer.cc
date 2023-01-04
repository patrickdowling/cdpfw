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

#include "command_tokenizer.h"

#include <ctype.h>

namespace util {

// There are multiple strategies for parsing:
// - Collect a whole line, then tokenize, which we do here for "reasons".
// - Tokenize on the fly; this means maintaining more persistent state.
//
// Ideally it'd be nice to not traverse the input buffer N times (i.e. tokenize and match commands
// on-the-fly) but that might requires a second/third attempt.
//
// The implementation isn't exactly optimized for minimal buffer use, and much like strtok we're
// poking around in the line buffer directly.

const char *CommandTokenizer::tokens_[CommandTokenizer::kMaxTokens] = {nullptr};

CommandTokenizer::Tokens CommandTokenizer::Tokenize(char *line)
{
  char *pos = line;

  uint8_t num_tokens = 0;
  bool eol = false;
  do {
    // skip whitespace
    while (*pos && isspace(*pos)) ++pos;

    // next token up to whitespace or #
    auto token = pos;
    while (*pos) {
      auto c = *pos;
      if ('#' == c) {
      // Comments abort further scanning, but there may exist a token ("token#comment")
        *pos = 0;
        eol = true;
      } else if (isspace(c)) {
        // Spaces end token (\n will probably have been chopped at input)
        *pos = 0;
      } else {
        ++pos;
      }
    }
    if (*token && num_tokens < kMaxTokens)
      tokens_[num_tokens++] = token;
    else
      eol = true;
    ++pos;

  } while (*pos && !eol);

  for (auto n = num_tokens; n < kMaxTokens; ++n) tokens_[n] = nullptr;

  return { num_tokens, tokens_ };
}

}  // namespace util
