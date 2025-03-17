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
 * src/c4/src/block --
 *   
 */

#include <numeric>
#include <ranges>

#include <c4/block.hxx>
#include <c4/ast/block_expression.hxx>

#include <c4/evaluation_stack.hxx>
#include <c4/ast/expression.hxx>

bool
c4::block::load_parameters(std::span<const ast::expression*> args,
                           std::shared_ptr<evaluation_stack>& my_stack) const {
    if (!ast_block) return false;

    for (const auto [arg, symbol]: std::ranges::zip_view(args,
                                                         ast_block->parameters.symbols)) {
        my_stack->set(symbol::from_ast(symbol),
                      arg);
    }
    return true;
}

c4::value
c4::block::do_evaluate(const std::shared_ptr<evaluation_stack>& my_stack) const {
    return std::accumulate(ast_block->exprs.begin(), ast_block->exprs.end(),
                           value::nil(),
                           [&my_stack]<typename Last>(Last&& acc,
                                                      const auto& expr) {
                               std::ignore = std::forward<Last>(acc);
                               return expr.evaluate(my_stack);
                           });
}

void
c4::block::print(std::ostream& os) const {
    if (ast_block) {
        os << "(block@" << ast_block << ")";
    }
    else {
        os << "{}";
    }
}

bool
c4::native_block::load_parameters(std::span<const ast::expression*> args,
                                  std::shared_ptr<evaluation_stack>& my_stack) const {
    for (size_t i = 0; i < args.size(); ++i) {
        my_stack->set(symbol::argument(i), args[i]);
    }
    return true;
}

c4::value
c4::native_block::do_evaluate(const std::shared_ptr<evaluation_stack>& my_stack) const {
    return _impl_fn(my_stack);
}

c4::value
c4::block::evaluate(const std::shared_ptr<evaluation_stack>& stack,
                    const std::span<const ast::expression*> args) const {
    auto my_stack = stack->push();
    if (!load_parameters(args, my_stack)) return value::nil();
    if (_closure_stack) my_stack->merge(*_closure_stack);

    return do_evaluate(my_stack);
}

void
c4::native_block::print(std::ostream& os) const {
    os << "(native-block@" << this << ")";
}
