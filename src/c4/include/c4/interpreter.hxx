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
 * src/c4/include/c4/interpreter --
 *   
 */
#ifndef INTERPRETER_HXX
#define INTERPRETER_HXX

#include <vector>
#include <ostream>
#include <iostream>

#include <c4/ast.hxx>
#include <c4/evaluation_stack.hxx>

namespace c4 {
    struct interpreter {
        explicit
        interpreter(std::ostream& error_stream = std::cerr)
            : _error_stream{error_stream}
            , _evaluation_stack{std::make_shared<evaluation_stack>()} { }

        bool
        parse(std::string_view str);

        bool
        parse(const std::string& str);

        int
        exec() const;

        void
        define(const std::string_view name,
               const int arity,
               std::unique_ptr<block, block_deleter> block) {
            _global_scope.define(ast::symbol(name, arity));
            _evaluation_stack->set(symbol(name, arity), std::move(block));
        }

    private:
        std::ostream& _error_stream;
        std::vector<ast::expression> _expressions{};
        ast::symbol_scope _global_scope{};
        std::shared_ptr<evaluation_stack> _evaluation_stack{};
    };
}

#endif
