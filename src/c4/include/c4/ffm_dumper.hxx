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
 * src/c4/include/c4/ffm_dumper --
 *   
 */
#ifndef BLAST_FFM_DUMPER_HXX
#define BLAST_FFM_DUMPER_HXX

#include <ostream>
#include <span>

#include <c4/visitor/visitor.hxx>
#include <c4/ffm/ffm_context.hxx>

namespace c4 {
    struct ffm_dumper final : ast2::visitor<
                ffm::function,
                ffm::function_call,
                ffm::function_declaration,
                ffm::function_definition,
                ffm::function_pack,
                ffm::unpack,
                ffm::block_argument,
                ffm::literal,
                ffm::block_literal,
                ffm::local,
                ffm::local_ref,
                ffm::context_access,
                ffm::context_object
            > {
        explicit
        ffm_dumper(std::ostream& os)
            : _os{os} { }

        void do_visit(const ffm::function& obj) override;

        void do_visit(const ffm::function_call& obj) override;

        void do_visit(const ffm::function_pack& obj) override;

        void do_visit(const ffm::function_declaration& obj) override;

        void do_visit(const ffm::function_definition& obj) override;

        void do_visit(const ffm::unpack& obj) override;

        void do_visit(const ffm::block_argument& obj) override;

        void do_visit(const ffm::literal& obj) override;

        void do_visit(const ffm::local& obj) override;

        void do_visit(const ffm::local_ref& obj) override;

        void do_visit(const ffm::context_access& obj) override;

        void do_visit(const ffm::context_object& obj) override;

        void do_visit(const ffm::block_literal& obj) override;

    private:
        void
        print_call_like(std::string_view call_type,
                        const ffm::function_declaration* obj,
                        std::span<ffm::value_expression* const> args);

        std::ostream& _os;
        char _call_end{'\n'};
    };
}

#endif
