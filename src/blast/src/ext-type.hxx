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
 * Originally created: 2026-08-10.
 *
 * src/blast/src/ext-type --
 *		A set of functions providing identifiers for specific blAST specific
 *		types external to C4.
 *
 *		The following are provided:
 *			- ast_unit -> clang::ASTUnit**
 *			- handler -> bst::handler_base*
 */
#ifndef BLAST_EXT_TYPE_HXX
#define BLAST_EXT_TYPE_HXX

#include <stdint.h>

#include <c4rt3/c4rt.h>

namespace bst {
	struct handler_base;
}

c4_extern uint32_t
blast_typeid_ast_unit();

#define \
c4_datum_from_ast_unit(ast_unit, out) \
	c4_datum_from_external(ast_unit, blast_typeid_ast_unit(), out)

#define \
c4_datum_get_ast_unit(datum, out) \
	c4_datum_get_external_typed(datum, clang::ASTUnit**, blast_typeid_ast_unit(), out)

c4_extern uint32_t
blast_typeid_handler();

#define \
c4_datum_from_handler(printer, out) \
	c4_datum_from_external(printer, blast_typeid_handler(), out)

#define \
c4_datum_get_handler(datum, out) \
	c4_datum_get_external_typed(datum, bst::handler_base*, blast_typeid_handler(), out)

#endif
