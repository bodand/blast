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
 * Originally created: 2025-03-03.
 *
 * src/c4/src/ast2/block --
 *
 */

#include <iostream>

#include <c4/ast2/block.hxx>
#include <c4/ast2/expression.hxx>
#include <c4/visitor/visitor.hxx>

#include <libassert/assert.hpp>
#include <utility>

c4::ast2::block_args::block_args(const c4::position& position,
                                 const std::span<symbol> args)
    : source_positioned{position}
    , _args{args.begin(), args.end()} { }

c4::ast2::block_argument&
c4::ast2::block_args::argument_reference(std::size_t arg_idx) {
    DEBUG_ASSERT(arg_idx < _args.size());
    return _args[arg_idx];
}

const c4::ast2::block_argument&
c4::ast2::block_args::argument_reference(std::size_t arg_idx) const {
    DEBUG_ASSERT(arg_idx < _args.size());
    return _args[arg_idx];
}

c4::ast2::block::block(const c4::position& position,
                       std::vector<expression*>&& expressions,
                       block_args* args)
    : source_positioned{position}
    , _args(args)
    , _expressions{std::move(expressions)} {
    ASSERT(_args->size() < std::numeric_limits<unsigned>::max(),
           "too many arguments in block");
}

bool
c4::ast2::block::requires_context() const noexcept {
    return std::ranges::any_of(_expressions, [](const auto& expr) { return expr->closure(); });
}

namespace {
    template<class It>
    struct defined_symbols_remover final : c4::ast2::visitor<c4::ast2::let_expression> {
        defined_symbols_remover(It begin, It end)
            : begin(std::move(begin))
            , end(std::move(end)) { }

        void
        do_visit(const c4::ast2::let_expression& obj) override {
            // you'd think we need to recurse here, but this is not the case
            // since symbols defined in nested blocks are already filtered out
            // and even if we have symbols with the same name as defined down-er
            // they are different symbols shadowing the one we are copying into
            // context
            end = std::remove(begin, end, obj.symbol());
        }

        It begin;
        It end;
    };

    template<class It>
    defined_symbols_remover(It begin, It end) -> defined_symbols_remover<It>;

    std::vector<c4::ast2::symbol>
    merge_child_contexts(const std::span<c4::ast2::expression* const> expressions) {
        std::vector<c4::ast2::symbol> result;
        for (const auto& expr : expressions) result.append_range(expr->closure_symbols());
        return result;
    }

    void
    deduplicate(std::vector<c4::ast2::symbol>& result) {
        std::ranges::sort(result);
        const auto [dup_begin, dup_end] = std::ranges::unique(result);
        result.erase(dup_begin, dup_end);
    }

    void
    remove_block_arguments(std::vector<c4::ast2::symbol>& result,
                           const c4::ast2::block_args* args) {
        if (!args) return;
        for (const auto& arg : args->args()) std::erase(result, *arg);
    }

    void
    remove_local_symbols(std::vector<c4::ast2::symbol>& result,
                         const std::span<c4::ast2::expression* const> expressions) {
        defined_symbols_remover remover(result.begin(), result.end());
        for (const auto& expr : expressions) expr->accept_skip_self(remover);
        result.erase(remover.end, result.end());
    }

    void
    remove_locally_defined(std::vector<c4::ast2::symbol>& result,
                           const c4::ast2::block_args* args,
                           const std::span<c4::ast2::expression* const> expressions) {
        remove_block_arguments(result, args);
        remove_local_symbols(result, expressions);
    }
}

std::vector<c4::ast2::symbol>
c4::ast2::block::effective_context_symbols() const {
    auto result = merge_child_contexts(_expressions);
    deduplicate(result);
    remove_locally_defined(result, _args, _expressions);
    return result;
}

std::span<const c4::ast2::expression* const>
c4::ast2::block::expressions() const {
    return std::span(_expressions);
}
