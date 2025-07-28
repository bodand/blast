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
 * src/c4/include/c4/ffm/symbol --
 *   
 */
#ifndef FFM_SYMBOL_HXX
#define FFM_SYMBOL_HXX

#include <c4/tags/source_positioned.hxx>
#include <c4/tags/visitable.hxx>

#include <c4/ast2/symbol.hxx>

namespace c4::ffm {
  struct symbol : ast2::tags::visitable
                , ast2::tags::source_positioned {
      static symbol
      from_ast(const ast2::symbol& ast_sym) {
          return {ast_sym.position(), ast_sym.length(),
                  ast_sym.name(), ast_sym.arity()};
      }

      symbol(const c4::position& position,
             const std::size_t length,
             const std::string_view& name,
             const unsigned arity)
              : source_positioned{position, length}, _name{name}, _arity{arity} { }

      [[nodiscard]] std::string_view
      name() const { return _name; }

      [[nodiscard]] unsigned
      arity() const { return _arity; }

  private:
      std::string_view _name;
      unsigned _arity;
  };
}

#endif
