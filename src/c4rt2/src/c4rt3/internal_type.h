/* blAST project
 *
 * Copyright (c) 2026 András Bodor <bodand@pm.me>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * - Redistributions of source code must retain the above copyright notice, this
 *   list of conditions and the following disclaimer.
 *
 * - Redistributions in binary form must reproduce the above copyright notice,
 *   this list of conditions and the following disclaimer in the documentation
 *   and/or other materials provided with the distribution.
 *
 * - Neither the name of the copyright holder nor the names of its contributors
 *   may be used to endorse or promote products derived from this software
 *   without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * Originally created: 2026-07-31.
 *
 * src/c4rt2/src/c4rt3/internal_type --
 *   Defines the PImpl's implementation part to be used within c4rt3. MUST BE
 *   KEPT IN ORDER WITH c4rt2c's LLVM IR structure and its usage.
 */
#ifndef BLAST_INTERNAL_TYPE_H
#define BLAST_INTERNAL_TYPE_H

#include <c4rt3/c4rt.h>

struct c4_datum_t {
	int32_t type;
	int32_t argv_sz;

	union {
		void* val_ptr;
		int64_t val_int64;
		double val_float64;
	};

	union {
		struct c4_datum_t** argv;
		uint64_t str_sz;
	};
};

#define datum_blk(d) ((d)->val_ptr)
#define datum_str(d) ((d)->val_ptr)
#define datum_int(d) ((d)->val_int64)
#define datum_flt(d) ((d)->val_float64)

#define datum_str_sz(d) ((d)->str_sz)
#define datum_sstr(d) datum_str(d), datum_str_sz(d)

#endif
