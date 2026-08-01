/* Original source from skalibs at
 * https://git.skarnet.org/cgit/skalibs/tree/src/libstddjb/uint64_fmt_generic.c
 * https://git.skarnet.org/cgit/skalibs/tree/src/libstddjb/fmtscan_asc.c
 *
 * "Acquired" on 2026-08-01. License text follows, after that the file modified
 * to work in this environment where only a select few functions are taken.
 *
 * Code-style changed to fit the project.
 */

/*
Copyright (c) 2011-2026 Laurent Bercot <ska-skaware@skarnet.org>

Permission to use, copy, modify, and distribute this software for any
purpose with or without fee is hereby granted, provided that the above
copyright notice and this permission notice appear in all copies.

THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
*/

/* ISC license. */

#include <stdint.h>
#include <stdlib.h>

#include "fmt.h"

static unsigned char
fmtscan_asc(const unsigned char c) {
	static char const* tab = "0123456789abcdefghijklmnopqrstuvwxyz";
	return (c >= 36) ? 0 : tab[c];
}

size_t
uint64_fmt_generic(char* fmt, uint64_t d, const uint8_t base) {
	size_t len = 1;
	uint64_t q = d;
	while (q >= base) {
		len++;
		q /= base;
	}
	if (fmt) {
		fmt += len;
		do {
			*--fmt = (char)fmtscan_asc(d % base);
			d /= base;
		} while (d);
	}
	return len;
}
