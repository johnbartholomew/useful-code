#ifndef UTF8_H
#define UTF8_H

/*
 * Decoder:
 *   Copyright (c) 2008-2009 Bjoern Hoehrmann <bjoern@hoehrmann.de>
 *   See http://bjoern.hoehrmann.de/utf-8/decoder/dfa/ for details.
 *
 * Encoder:
 *   Copyright (c) 2012 John Bartholomew <jpa.bartholomew@gmail.com>
 *   See any UTF-8 spec and basic programming knowledge for details.
 *
 * Both:
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include <stdint.h>
#include <assert.h>

#define UTF8_ACCEPT 0
#define UTF8_REJECT 12

inline uint32_t utf8_decode(uint32_t *state, uint32_t *codep, uint32_t byte) {
   extern const uint8_t UTF8_DECODE_DFA[];
   uint32_t type = UTF8_DECODE_DFA[byte];
   *codep = (*state != UTF8_ACCEPT) ?
      (byte & 0x3fu) | (*codep << 6) :
      (0xff >> type) & (byte);
   return (*state = UTF8_DECODE_DFA[256 + *state + type]);
}

inline int utf8_encode(uint8_t *out, uint32_t codep) {
   if (codep <= 0x7Fu) {
      out[0] = codep;
      return 1;
   } else if (codep <= 0x7FFu) {
      out[0] = 0xC0u | ((codep & 0x7C0u) >> 6);
      out[1] = 0x80u | (codep & 0x3Fu);
      return 2;
   } else if (codep <= 0xFFFFu) {
      /* not a surrogate pair code-point */
      assert(codep < 0xD800u || codep > 0xDFFFu);
      out[0] = 0xE0u | ((codep & 0xF000u) >> 12);
      out[1] = 0x80u | ((codep & 0x0FC0u) >>  6);
      out[2] = 0x80u | ((codep & 0x003Fu) >>  0);
      return 3;
   } else {
      /* valid Unicode code-point */
      assert(codep <= 0x10FFFFu);
      out[0] = 0xF0u | ((codep & 0x1C0000u) >> 18);
      out[1] = 0x80u | ((codep & 0x03F000u) >> 12);
      out[2] = 0x80u | ((codep & 0x000FC0u) >>  6);
      out[3] = 0x80u | ((codep & 0x00003Fu) >>  0);
      return 4;
   }
}

#endif
