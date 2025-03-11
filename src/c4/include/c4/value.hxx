/* demo project
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
 * Originally created: 2025-02-26.
 *
 * src/c4/value --
 *   
 */
#ifndef VALUE_HXX
#define VALUE_HXX

#include <string>
#include <variant>
#include <cstdint>
#include <memory>
#include <ostream>
#include <span>

#include <c4/symbol.hxx>

namespace c4 {
    namespace ast {
        struct block_expression;
        struct expression;
    }

    struct block;
    struct evaluation_stack;

    struct block_deleter {
        constexpr block_deleter() = default;

        void operator()(block* b) const;
    };

    struct value {
        explicit(false)
        value(std::int64_t i64)
            : impl(i64) { }

        explicit(false)
        value(double d)
            : impl(d) { }

        explicit(false)
        value(std::string s)
            : impl(s) { }

        explicit(false)
        value(symbol s)
            : impl(s) { }

        explicit(false)
        value(std::unique_ptr<block, block_deleter>&& b);

        [[nodiscard]] value
        evaluate(evaluation_stack& stk, std::span<const ast::expression*> args) const;

        [[nodiscard]] static value
        from_block_ast(const ast::block_expression* blk_expr);

        [[nodiscard]] static value
        nil();

        friend std::ostream&
        operator<<(std::ostream& os, const value& obj);

        std::int64_t coerce_to_int() const;

        // double coerce_to_double() const;
        //
        // std::string coerce_to_string() const;
        //
        // symbol coerce_to_symbol() const;

    private:
        std::variant<
            std::int64_t,
            double,
            std::string,
            symbol,
            std::unique_ptr<block, block_deleter>
        > impl;
    };
}

#endif
