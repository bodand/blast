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
 * Originally created: 2025-02-17.
 *
 * src/parser/ast --
 *   
 */
#ifndef DEMO_AST_HXX
#define DEMO_AST_HXX

#include <boost/fusion/include/io.hpp>
#include <boost/spirit/home/x3/support/ast/variant.hpp>
#include <boost/spirit/home/x3/support/ast/position_tagged.hpp>
#include <boost/spirit/home/x3.hpp>

#include <string>
#include <utility>
#include <vector>
#include <unordered_map>

namespace demo::ast {
  namespace x3 = boost::spirit::x3;

  struct block_expression;
  struct let_expression;
  struct symbol_expression;

  struct symbol : x3::position_tagged {
      std::string name;
      std::size_t arity{};

      symbol(std::string name = {}, size_t arity = {})
              : name(std::move(name)),
                arity(arity) { }
  };

  struct expression
          : x3::variant<
                  std::string,
                  x3::forward_ast<let_expression>,
                  x3::forward_ast<block_expression>,
                  x3::forward_ast<symbol_expression>
          >,
            x3::position_tagged {
      using base_type::base_type;
      using base_type::operator=;
  };

  struct let_expression : x3::position_tagged {
      std::string symbol;
      expression expr;

      let_expression(const std::string& symbol = {},
                     const expression& expr = {})
              : symbol(symbol),
                expr(expr) { }
  };

  struct symbol_expression : x3::position_tagged {
      std::string symbol;
      std::vector<expression> arguments;

      symbol_expression(const std::string& symbol = {},
                        const std::vector<expression>& arguments = {})
              : symbol(symbol),
                arguments(arguments) { }
  };

  struct block_parameters : x3::position_tagged {
      std::vector<symbol> parameters;

      block_parameters(const std::vector<symbol>& parameters = {})
              : parameters(parameters) { }

      [[nodiscard]] auto
      arity() const noexcept { return parameters.size(); }
  };

  struct block_expression : x3::position_tagged {
      block_parameters parameters;
      std::vector<expression> expressions;

      block_expression(const block_parameters& parameters = {},
                       const std::vector<expression>& expressions = {})
              : parameters(parameters),
                expressions(expressions) { }

      [[nodiscard]] auto
      arity() const noexcept { return parameters.arity(); }
  };

  struct script : x3::position_tagged {
      std::vector<expression> expressions;

      script(const std::vector<expression>& expressions = {})
              : expressions(expressions) { }
  };

  using boost::fusion::operator<<;

  struct symbol_table_tag;
  struct position_cache_tag;

  struct symbol_table {
      x3::symbols<std::string> parser;
      std::unordered_map<std::string, symbol> symbols;

      void
      add_symbol(std::string name, std::size_t arity) {
          parser.add(name, name);
          symbols.try_emplace(name,
                              name, arity);
      }

      [[nodiscard]] std::size_t
      arity_of(const std::string& symbol) {
          if (const auto it = symbols.find(symbol);
                  it != symbols.end()) {
              return it->second.arity;
          }
#ifndef NDEBUG
          throw std::runtime_error("fatal: unknown symbol parsed in call: " + symbol);
#endif
          std::unreachable();
      }
  };
}

#endif
