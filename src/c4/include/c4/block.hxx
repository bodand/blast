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
 * Originally created: 2025-03-03.
 *
 * src/c4/include/c4/block --
 *   
 */
#ifndef BLOCK_HXX
#define BLOCK_HXX

#include <span>
#include <functional>
#include <vector>

#include <c4/ast_fwd.hxx>
#include <c4/evaluation_stack.hxx>
#include <c4/value.hxx>
#include <c4/ast/block_expression.hxx>

namespace c4 {
    struct block {
        explicit
        block(const ast::block_expression* ast_block,
              std::shared_ptr<evaluation_stack> stk = {})
            : ast_block{ast_block}
            , _closure_stack{std::move(stk)} {
            // make block nil if given block is already nil
            if (ast_block && ast_block->exprs.empty()) this->ast_block = nullptr;
        }

        [[nodiscard]] value
        evaluate(const std::shared_ptr<evaluation_stack>& stack, std::span<const ast::expression*> args) const;

        virtual void
        print(std::ostream& os) const;

        [[nodiscard]] virtual bool
        is_nil() const noexcept { return ast_block == nullptr; }

        virtual ~block() = default;

    protected:
        virtual value
        do_evaluate(const std::shared_ptr<evaluation_stack>& my_stack) const;

        virtual bool
        load_parameters(std::span<const ast::expression*> args,
                        std::shared_ptr<evaluation_stack>& my_stack) const;

    private:
        const ast::block_expression* ast_block;
        std::shared_ptr<evaluation_stack> _closure_stack;

        friend bool
        operator==(const block& lhs, const block& rhs);

        friend bool
        operator!=(const block& lhs, const block& rhs) { return !(lhs == rhs); }
    };

    struct native_block final : block {
        template<class Fn>
            requires std::invocable<Fn, std::shared_ptr<evaluation_stack>&>
        explicit
        native_block(Fn&& fn)
            : block{nullptr}
            , _impl_fn{std::forward<Fn>(fn)} { }

        void
        print(std::ostream& os) const override;

        [[nodiscard]] bool
        is_nil() const noexcept override { return false; }

    protected:
        value
        do_evaluate(const std::shared_ptr<evaluation_stack>& my_stack) const override;

        bool load_parameters(std::span<const ast::expression*> args,
                             std::shared_ptr<evaluation_stack>& my_stack) const override;

    private:
        std::function<value(const std::shared_ptr<evaluation_stack>&)> _impl_fn;
    };

    inline bool
    operator==(const block& lhs, const block& rhs) {
        if (lhs.ast_block == nullptr) return rhs.ast_block == nullptr;
        return false;
    }
}

#endif
