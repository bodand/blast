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
 * Originally created: 2025-06-06.
 *
 * src/c4/include/c4/ffm/function --
 *   
 */
#ifndef FFM_FUNCTION_HXX
#define FFM_FUNCTION_HXX

#include <utility>
#include <optional>
#include <variant>

#include <c4/tags/referable.hxx>
#include <c4/tags/visitable.hxx>
#include <c4/ffm/symbol.hxx>
#include <c4/ffm/ffm_node.hxx>

namespace c4::ffm {
    struct function_declaration;
    struct function_definition;

    struct function final : ffm_node
                            , ast2::tags::visitable
                            , ast2::tags::referable {
        using value_type = std::variant<
            function_declaration*,
            function_definition*>;

        explicit function(value_type value);

        std::string_view
        name() const override;

        unsigned
        base_arity() const override;

        unsigned
        effective_arity() const override;

        template<class V>
        void
        accept_skip_self(V&& visitor) const {
            std::visit([&v = std::forward<V>(visitor)]<class T>(T&& val) mutable {
                std::forward<T>(val)->accept(v);
            }, _value);
        }

    private:
        value_type _value;
    };
}

#endif
