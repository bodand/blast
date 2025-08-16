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
 * src/c4/include/c4/ffm/expression --
 *   value_expression can be passed to other function calls/packs as an
 *   argument; it is either a packed literal, argument, or packed fn. call
 *
 *   root_expression can stand at a root from a block's body; it is either a
 *   a packed literal, argument, unpack, or raw fn. call.
 *   A packed literal or argument in non-last position is for all intents and
 *   purposes useless and may be discarded during ffm generation.
 */
#ifndef BLAST_EXPRESSION_HXX
#define BLAST_EXPRESSION_HXX

#include <variant>

#include <c4/ffm/ffm_node.hxx>

namespace c4::ffm {
    struct function_call;
    struct block_argument;
    struct function_pack;
    struct unpack;
    struct context_object;
    struct context_access;
    struct literal;
    struct block_literal;
    struct local;
    struct local_ref;

    struct value_expression final : ffm_node {
        using value_type = std::variant<literal*,
                                        block_literal*,
                                        block_argument*,
                                        function_pack*,
                                        local_ref*,
                                        context_access*,
                                        context_object*>;

        explicit
        value_expression(const value_type& value)
            : _value{value} { }

        template<class V>
        void
        accept_skip_self(V&& visitor) const {
            std::visit([&v = std::forward<V>(visitor)]<class T>(T&& val) mutable {
                std::forward<T>(val)->accept(v);
            }, _value);
        }

        [[nodiscard]] const value_type&
        value() const noexcept { return _value; }

    private:
        value_type _value;
    };

    struct root_expression final : ffm_node {
        using value_type = std::variant<literal*,
                                        block_literal*,
                                        block_argument*,
                                        function_call*,
                                        unpack*,
                                        local*>;

        explicit
        root_expression(const value_type& value)
            : _value{value} { }

        [[nodiscard]] bool
        discardable_nonlast() const noexcept;

        template<class V>
        void
        accept_skip_self(V&& visitor) const {
            std::visit([&v = std::forward<V>(visitor)]<class T>(T&& val) mutable {
                std::forward<T>(val)->accept(v);
            }, _value);
        }

        [[nodiscard]] const value_type&
        value() const noexcept { return _value; }

    private:
        value_type _value;
    };
}

#endif
