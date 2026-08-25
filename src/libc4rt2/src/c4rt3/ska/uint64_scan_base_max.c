/* Original source from skalibs at
 * https://git.skarnet.org/cgit/skalibs/tree/src/libstddjb/uint64_scan_base_max.c
 * https://git.skarnet.org/cgit/skalibs/tree/src/libstddjb/fmtscan_num.c
 *
 * "Acquired" on 2026-08-08. License text follows, after that the file modified
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

#include <stddef.h>
#include <stdint.h>

#include "scan.h"

static unsigned char
fmtscan_num(const unsigned char c, const unsigned char n) {
	return
			((c < '0') || (n > 36))
				? n
				: (n <= 10)
					  ? (c - '0' <= n)
						    ? c - '0'
						    : n
					  : (c - '0' <= 9)
						    ? c - '0'
						    : (c < 'A')
							      ? n
							      : (c - 'A' < n - 10)
								        ? c - 'A' + 10
								        : (c < 'a')
									          ? n
									          : (c - 'a' < n - 10)
										            ? c - 'a' + 10
										            : n;
}

size_t
uint64_scan_base_max(char const* s,
                     uint64_t* u,
                     const uint8_t base,
                     const uint64_t max) {
	uint64_t result = 0;
	size_t pos = 0;
	for (;; pos++) {
		const uint8_t c = fmtscan_num(s[pos], base);
		if (c >= base || result > ((max - c) / base)) break ;
		result = result * base + c;
	}
	if (pos) *u = result;
	return pos;
}
