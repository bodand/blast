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
 * src/c4/include/c4/ffm/context_type --
 *   
 */
#ifndef BLAST_CONTEXT_TYPE_HXX
#define BLAST_CONTEXT_TYPE_HXX

#include <span>
#include <string>
#include <utility>
#include <vector>

#include <c4/tags/attributable.hxx>
#include <c4/tags/visitable.hxx>

#include <c4/ffm/block_argument.hxx>
#include <c4/ffm/ffm_node.hxx>
#include <c4/ffm/symbol.hxx>

namespace c4::ffm {
    struct value_expression;
}

namespace c4::ffm {
    struct local_ref;

    struct context_type final : ffm_node
                                , ast2::tags::attributable
                                , ast2::tags::visitable {
        context_type(std::string name,
                     std::vector<ast2::tags::referable*> fields);

        [[nodiscard]] std::string
        name() const { return _name; }

        void
        name(const std::string_view name) { _name = name; }

        [[nodiscard]] std::span<ast2::tags::referable* const>
        fields() const;

        [[nodiscard]] bool
        contains(const ast2::tags::referable* ref) const noexcept;

    private:
        std::string _name;
        std::vector<ast2::tags::referable*> _fields;
    };

    struct context_object final : ffm_node
                                  , ast2::tags::visitable {
        explicit
        context_object(context_type* ctx_type)
            : _ctx_type{ctx_type} { }

        [[nodiscard]] context_type*
        ctx_type() const noexcept { return _ctx_type; }

        [[nodiscard]] std::span<value_expression* const>
        args() const noexcept { return _args; }

        void
        push_argument(value_expression* ref) { _args.push_back(ref); }

    private:
        context_type* _ctx_type;
        std::vector<value_expression*> _args{};
    };

    struct context_access final : ffm_node
                                  , ast2::tags::visitable {
        context_access(block_argument* arg, context_type* ctx_type, const std::string_view name);

        [[nodiscard]] context_type*
        ctx_type() const noexcept { return _ctx_type; }

        [[nodiscard]] std::string_view
        name() const noexcept { return _name; }

        [[nodiscard]] block_argument*
        arg() const noexcept { return _arg; }

    private:
        context_type* _ctx_type;
        block_argument* _arg;
        std::string_view _name;
    };
}

#endif
