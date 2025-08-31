/* blAST project
 *
 * Copyright (c) 2025 András Bodor <bodand@pm.me>
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
 * Originally created: 2025-08-08.
 *
 * src/c4/src/ffm/attributes --
 *   Collection of attributes used by ffm mapping.
 */
#ifndef BLAST_ATTRIBUTES_HXX
#define BLAST_ATTRIBUTES_HXX

#include <c4/tags/attributable.hxx>

namespace c4::ast2 {namespace tags {
        struct referable;
    }

    struct symbol;
}

namespace c4::ffm {
    struct function_declaration;
    struct local;
    struct block_argument;

    struct declaration_attribute final : ast2::tags::typed_attribute<function_declaration*> {
        explicit
        declaration_attribute(function_declaration* value)
            : typed_attribute{value} { }
    };

    struct argument_attribute final : ast2::tags::typed_attribute<block_argument*> {
        explicit
        argument_attribute(block_argument* value)
            : typed_attribute{value} { }
    };

    struct local_attribute final : ast2::tags::typed_attribute<local*> {
        explicit
        local_attribute(local* value)
            : typed_attribute{value} { }
    };

    block_argument*
    try_get_referenced_argument(const ast2::symbol& sym);

    local*
    try_get_referenced_local(const ast2::symbol& sym);

    function_declaration*
    try_get_declaration(const ast2::symbol& sym, std::string_view name = "declaration");

    function_declaration*
    try_get_declaration(const ast2::tags::referable& ref, std::string_view name = "declaration");
}

#endif
