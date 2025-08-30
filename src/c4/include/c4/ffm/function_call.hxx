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
 * src/c4/include/c4/ffm/function_call --
 *   
 */
#ifndef FFM_FUNCTION_CALL_HXX
#define FFM_FUNCTION_CALL_HXX

#include <c4/tags/source_positioned.hxx>
#include <c4/tags/visitable.hxx>

#include <c4/ffm/ffm_node.hxx>
#include <c4/ffm/function_declaration.hxx>
#include <c4/ffm/expression.hxx>

namespace c4::ffm {
    struct argument_holder {
        argument_holder(const argument_holder& other) = delete;

        argument_holder&
        operator=(const argument_holder& other) = delete;

        argument_holder(argument_holder&& other) noexcept = delete;

        argument_holder&
        operator=(argument_holder&& other) noexcept = delete;

        virtual ~argument_holder() = default;

        [[nodiscard]] std::span<value_expression* const>
        arguments() const { return _arguments; }

        void
        push_argument(value_expression* expr) { _arguments.push_back(expr); }

        [[nodiscard]] bool
        packed() const noexcept { return _packed; }

    protected:
        explicit
        argument_holder(const bool packed)
            : _packed{packed} { }

    private:
        bool _packed{};
        std::vector<value_expression*> _arguments{};
    };

    struct function_call final : ffm_node
                                 , ast2::tags::visitable
                                 , ast2::tags::source_positioned
                                 , argument_holder {
        function_call(const c4::position& position,
                      function_declaration* const fn,
                      const bool packed = false)
            : source_positioned{position}
            , argument_holder{packed}
            , _fn{fn} { }

        [[nodiscard]] const function_declaration*
        function() const noexcept { return _fn; }

    private:
        function_declaration* _fn;
    };

    struct dynamic_call final : ffm_node
                                , ast2::tags::source_positioned
                                , ast2::tags::visitable
                                , argument_holder {
        explicit
        dynamic_call(const c4::position& position,
                     value_expression* callee,
                     const bool packed = false)
            : source_positioned{position}
            , argument_holder{packed}
            , _callee{callee} { }

        [[nodiscard]] const value_expression*
        callee() const { return _callee; }

    private:
        value_expression* _callee;
    };
}

#endif
