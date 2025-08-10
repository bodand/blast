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
 * src/c4/src/ffm_mapper --
 *   
 */

#include <c4/ffm_mapper.hxx>

#include <c4/ffm/ffm_context.hxx>
#include <c4/ffm/function.hxx>
#include <c4/ffm/function_call.hxx>
#include <c4/ffm/function_declaration.hxx>
#include <c4/ffm/function_definition.hxx>
#include <c4/ffm/symbol.hxx>
#include <c4/ffm/ffm_node.hxx>

#include <c4/ast2/block.hxx>
#include <c4/ast2/dynamic_call.hxx>
#include <c4/ast2/expression.hxx>
#include <c4/ast2/float_literal.hxx>
#include <c4/ast2/fn_call.hxx>
#include <c4/ast2/integer_literal.hxx>
#include <c4/ast2/let_expression.hxx>
#include <c4/ast2/op_call.hxx>
#include <c4/ast2/string_literal.hxx>
#include <c4/ast2/symbol.hxx>
#include <c4/p2/lex/tokens.hxx>

c4::ffm_mapper::ffm_mapper(ffm::ffm_context& ffm_context)
    : _ffm_context{ffm_context} {
    const ffm::symbol main(position::pseudo_position(), "@main", 0, false);
    declare_function(main);
}

void
c4::ffm_mapper::do_visit(const ast2::let_expression& obj) {
    declare_function(ffm::symbol::from_ast(obj.symbol(), mangled_scope(obj.mangled_name())));

    const auto name = obj.mangled_name();
    _block_names.push_back(name);
    _skip_implicit_block_entry = true;

    obj.value().accept(*this);

    _skip_implicit_block_entry = false;
    _block_names.pop_back();
}

void
c4::ffm_mapper::do_visit(const ast2::block& obj) {
    const auto stack_memory = enter_block();
    auto named_block = false;
    // _block_names needs to be able to refer to this after the if's scope
    const auto name = next_block_name(obj.arity());

    if (!_skip_implicit_block_entry) {
        named_block = true;
        const auto block_sym = ffm::symbol(obj.position(),
                                           mangled_scope(name),
                                           obj.arity(),
                                           _implicit_block_entry_closure);
        _block_names.push_back(name);
        declare_function(block_sym);
    }
    _skip_implicit_block_entry = false;


    for (const auto& expression : obj.expressions()) {
        expression->accept(*this);
    }

    if (named_block) _block_names.pop_back();
}

void
c4::ffm_mapper::do_visit(const ast2::block_args& obj) { }

void
c4::ffm_mapper::do_visit(const ast2::expression& obj) {
    _implicit_block_entry_closure = false;
    if (obj.closure()) {
        for (const auto& sym : obj.closure_symbols()) {
            if (sym.references()) {
                // a referenced symbol means we are a proper closure, thus
                // it needs to be passed down to blocks if we happen to have
                // one nested
                _implicit_block_entry_closure = true;
            }
            else {
                // referenced symbols are defined in the source file, that is they
                // will be found in let expressions, where we can properly name them
                // only global, extern functions need to be implicitly declared here
                // because of this
                declare_function(ffm::symbol::from_ast(sym));
            }
        }
    }
    obj.accept_skip_self(*this);
}

namespace {
    struct declaration_finder_visitor final : c4::ast2::visitor<c4::ffm::function_declaration> {
        explicit
        declaration_finder_visitor(const c4::ffm::symbol& sym)
            : symbol{sym} { }

        void
        do_visit(const c4::ffm::function_declaration& obj) override {
            if (!(obj.name() == symbol.name() && obj.base_arity() == symbol.arity()))
                return; // symbol does not match

            result = const_cast<c4::ffm::function_declaration*>(&obj);
        }

        c4::ffm::function_declaration* result{};
        c4::ffm::symbol symbol;
    };
}

c4::ffm::function_declaration*
c4::ffm_mapper::find_function_declaration(const ffm::symbol& sym) const {
    for (auto finder = declaration_finder_visitor{sym};
         const auto& root : _roots) {
        root->accept_skip_self(finder);
        if (finder.result) return finder.result;
    }
    return nullptr;
}

void
c4::ffm_mapper::declare_function(const ffm::symbol& sym) {
    if (find_function_declaration(sym)) return;

    const auto declaration = _ffm_context.build_function_declaration(sym);
    const auto fun = _ffm_context.build_function(declaration);
    _roots.push_back(fun);
}

c4::recursive_scope<c4::ffm::function_definition*>
c4::ffm_mapper::define_function(const ffm::symbol&) {
    const auto fn_def = _ffm_context.build_function_definition();
    const auto fn = _ffm_context.build_function(fn_def);
    _roots.push_back(fn);
    return recursive_scope(_current_function, fn_def);
}

namespace {
    std::size_t
    numeric_length(const unsigned num) {
        if (num == 0) return 1;
        return static_cast<std::size_t>(std::floor(std::log10(num)) + 1);
    }
}

std::string
c4::ffm_mapper::next_block_name(const unsigned arity) {
    const auto id = get_next_block_id();
    return fmt::format("{}#{}#{}",
                       1 + numeric_length(id) + 1 + numeric_length(arity),
                       id,
                       arity);
}
