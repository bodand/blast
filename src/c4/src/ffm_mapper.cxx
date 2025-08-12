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

#include <array>

#include <c4/ffm_mapper.hxx>

#include <c4/ffm/expression.hxx>
#include <c4/ffm/ffm_context.hxx>
#include <c4/ffm/ffm_node.hxx>
#include <c4/ffm/function.hxx>
#include <c4/ffm/function_call.hxx>
#include <c4/ffm/function_declaration.hxx>
#include <c4/ffm/function_definition.hxx>
#include <c4/ffm/function_pack.hxx>
#include <c4/ffm/symbol.hxx>

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
#include <c4/ffm/unpack.hxx>
#include <c4/p2/lex/tokens.hxx>

#include <libassert/assert.hpp>

namespace {
    struct declaration_attribute final : c4::ast2::tags::typed_attribute<c4::ffm::function_declaration*> {
        explicit
        declaration_attribute(c4::ffm::function_declaration* value)
            : typed_attribute{value} { }
    };

    struct argument_attribute final : c4::ast2::tags::typed_attribute<c4::ffm::block_argument*> {
        explicit
        argument_attribute(c4::ffm::block_argument* value)
            : typed_attribute{value} { }
    };

    struct definition_attribute final : c4::ast2::tags::typed_attribute<c4::ffm::function_definition*> {
        explicit
        definition_attribute(c4::ffm::function_definition* value)
            : typed_attribute{value} { }
    };
}

c4::ffm_mapper::ffm_mapper(ffm::ffm_context& ffm_context)
    : _ffm_context{ffm_context} {
    const ffm::symbol main(position::pseudo_position(), "@main", 0, false);

    const auto decl = declare_function(main, {});
    const auto fn_def = _ffm_context.build_function_definition(decl);
    const auto fn = _ffm_context.build_function(fn_def);
    _roots.push_back(fn);
    _current_function = fn_def;
}

void
c4::ffm_mapper::do_visit(const ast2::let_expression& obj) {
    _currently_in_let = &obj;

    obj.value().accept(*this);

    // TODO: enable after implementing proper handling of other nodes
    //  ASSERT(!_currently_in_let.has_value(),
    //      "entering let's value did not clear let entry",
    //      obj);
    _currently_in_let.reset();
}

void
c4::ffm_mapper::do_visit(const ast2::block& obj) {
    const auto stack_memory = enter_block();
    // _block_names needs to be able to refer to this after the if's scope
    std::string name;
    ffm::function_declaration* decl;

    auto arguments = make_argument_list(obj.args());

    if (_currently_in_let) {
        const auto let = _currently_in_let.value();
        DEBUG_ASSERT(let, "let is null");

        name = let->mangled_name();
        const auto block_sym = ffm::symbol::from_ast(
            let->symbol(),
            _closure != nullptr,
            mangled_scope(name));

        _block_names.push_back(name);
        decl = declare_function(block_sym, std::move(arguments));
        let->emplace_attribute<declaration_attribute>("declaration", decl);
    }
    else {
        name = next_block_name(obj.arity());
        const auto block_sym = ffm::symbol(obj.position(),
                                           mangled_scope(name),
                                           obj.arity(),
                                           _closure != nullptr);
        _block_names.push_back(name);
        decl = declare_function(block_sym, std::move(arguments));
        obj.emplace_attribute<declaration_attribute>("declaration", decl);
    }
    DEBUG_ASSERT(!name.empty(),
                 "name of function cannot be empty");

    // _currently_in_let needs to be cleared in all cases so no nested blocks
    // get named under the let object
    _currently_in_let.reset();

    // definition scope
    {
        const auto scope = define_function(decl);
        for (const auto& expression : obj.expressions()) {
            expression->accept(*this);
        }
    }

    // this pops both the anonymous block's name and the let's name, whichever
    // happened
    _block_names.pop_back();
}

void
c4::ffm_mapper::build_closure_context_from_symbols(const ast2::expression& obj) {
    std::vector<ffm::block_argument*> closure_symbols;
    for (const auto& sym : obj.closure_symbols()) {
        if (const auto ref = sym.references()) {
            // a referenced symbol means we are a proper closure, thus
            // it needs to be passed down to blocks if we happen to have
            // one nested...

            // ...except if the referenced symbol already has a fn. declaration
            // attached in which case it is a global function that is not a closure itself, meaning
            // it does not need to be captured, nor declared (as it is already done)
            if (const auto decl = ref->attribute_value<ffm::function_declaration*>("declaration");
                decl.has_value() && !(*decl)->closure())
                continue;

            closure_symbols.emplace_back(_ffm_context
                .build_block_argument(ref->position(), ref->name(), ref->base_arity()));
        }
        else {
            // referenced symbols are defined in the source file, that is they
            // will be found in let expressions, where we can properly name them
            // only global, extern functions need to be implicitly declared here
            // because of this
            // external functions cannot be closures, what could they be closed over
            // if they happen before anything in the given script happens?
            declare_extern_function(ffm::symbol::from_ast(sym, false));
        }
    }
    if (!closure_symbols.empty()) _closure = _ffm_context.build_context_type("anon", std::move(closure_symbols));
}

void
c4::ffm_mapper::do_visit(const ast2::expression& obj) {
    _closure = nullptr;
    if (obj.closure()) build_closure_context_from_symbols(obj);
    obj.accept_skip_self(*this);
}

c4::ffm::function_declaration*
c4::ffm_mapper::resolve_function_declaration(const ast2::symbol& sym) {
    ffm::function_declaration* decl;
    if (const auto ref = sym.references()) {
        const auto decl_opt = ref->attribute_value<ffm::function_declaration*>("declaration");
        ASSERT(decl_opt, "referenced entity must have a declaration", sym.name(), sym.base_arity());
        decl = *decl_opt;
    }
    else {
        decl = find_function_declaration(ffm::symbol::from_ast(sym, false));
    }

    ASSERT(decl, "called symbol must have a declaration", sym.name(), sym.base_arity());
    return decl;
}

c4::ffm::function_pack*
c4::ffm_mapper::build_function_pack_from_symbol(const position& position,
                                                const ast2::symbol& sym) {
    const auto decl = resolve_function_declaration(sym);
    return _ffm_context.build_function_pack(position, decl);
}

c4::ffm::function_call*
c4::ffm_mapper::build_function_call_from_symbol(const position& position,
                                                const ast2::symbol& sym) {
    const auto decl = resolve_function_declaration(sym);
    return _ffm_context.build_function_call(position, decl);
}

c4::ffm::value_expression*
c4::ffm_mapper::build_packed_function_call(const position& position,
                                           const ast2::symbol& sym,
                                           const std::span<const ast2::expression* const> args) {
    const auto call = build_function_pack_from_symbol(position, sym);
    const auto scope = enter_pack_arguments(call);

    for (const auto& arg : args) arg->accept(*this);

    return _ffm_context.build_value_expression(call);
}

c4::ffm::unpack*
c4::ffm_mapper::build_argument_unpack(ffm::block_argument* arg) {
    DEBUG_ASSERT(arg, "argument must not be null");
    const auto value = _ffm_context.build_value_expression(arg);
    return _ffm_context.build_unpack(value);
}

c4::ffm::root_expression*
c4::ffm_mapper::build_root_argument(ffm::block_argument* arg) {
    const auto unp_expr = build_argument_unpack(arg);
    return _ffm_context.build_root_expression(unp_expr);
}

c4::ffm::value_expression*
c4::ffm_mapper::build_value_argument(ffm::block_argument* arg) const {
    return _ffm_context.build_value_expression(arg);
}

c4::ffm::root_expression*
c4::ffm_mapper::build_root_function_call(const position& position,
                                         const ast2::symbol& sym,
                                         const std::span<const ast2::expression* const> args) {
    const auto call = build_function_call_from_symbol(position, sym);
    const auto scope = enter_call_arguments(call);

    for (const auto& arg : args) arg->accept(*this);

    return _ffm_context.build_root_expression(call);
}

namespace {
    c4::ffm::block_argument*
    try_get_referenced_argument(const c4::ast2::symbol& sym) {
        const auto ref = sym.references();
        if (!ref) return nullptr;

        const auto attr = ref->attribute_value<c4::ffm::block_argument*>("argument");
        if (!attr) return nullptr;

        return *attr;
    }
}

void
c4::ffm_mapper::push_call_argument_packed(const position& position,
                                          const ast2::symbol& sym,
                                          const std::span<const ast2::expression* const> args) {
    DEBUG_ASSERT(_current_call, "must be in a call on block root level (proper call)");

    if (const auto arg = try_get_referenced_argument(sym)) {
        const auto expr = build_value_argument(arg);
        _current_call->push_argument(expr);
    }
    else {
        ffm::value_expression* const expr = build_packed_function_call(position, sym, args);
        _current_call->push_argument(expr);
    }
}

void
c4::ffm_mapper::push_pack_argument_packed(const position& position,
                                          const ast2::symbol& sym,
                                          const std::span<const ast2::expression* const> args) {
    DEBUG_ASSERT(_current_pack, "must be in a pack in call argument (lazy call)");

    if (const auto arg = try_get_referenced_argument(sym)) {
        const auto expr = build_value_argument(arg);
        _current_pack->push_argument(expr);
    }
    else {
        ffm::value_expression* const expr = build_packed_function_call(position, sym, args);
        _current_pack->push_argument(expr);
    }
}

void
c4::ffm_mapper::push_root_call(const position& position,
                               const ast2::symbol& sym,
                               const std::span<const ast2::expression* const> args) {
    DEBUG_ASSERT(!_current_pack, "must not be in a pack in call argument (lazy call)");
    DEBUG_ASSERT(!_current_call, "must not be in a call on block root level (proper call)");
    DEBUG_ASSERT(_current_function, "must be in a function definition");

    if (const auto arg = try_get_referenced_argument(sym)) {
        const auto expr = build_root_argument(arg);
        _current_function->push_expression(expr);
    }
    else {
        const auto expr = build_root_function_call(position, sym, args);
        _current_function->push_expression(expr);
    }
}

void
c4::ffm_mapper::do_visit(const ast2::fn_call& obj) {
    if (_current_pack) return push_pack_argument_packed(obj.position(), obj.sym(), obj.args());
    if (_current_call) return push_call_argument_packed(obj.position(), obj.sym(), obj.args());
    push_root_call(obj.position(), obj.sym(), obj.args());
}

void
c4::ffm_mapper::do_visit(const ast2::binary_op_call& obj) {
    std::array<const ast2::expression* const, 2> args{&obj.left(), &obj.right()};
    if (_current_pack) return push_pack_argument_packed(obj.position(), obj.op(), args);
    if (_current_call) return push_call_argument_packed(obj.position(), obj.op(), args);
    push_root_call(obj.position(), obj.op(), args);
}

void
c4::ffm_mapper::do_visit(const ast2::unary_op_call& obj) {
    std::array<const ast2::expression* const, 1> args{&obj.operand()};
    if (_current_pack) return push_pack_argument_packed(obj.position(), obj.op(), args);
    if (_current_call) return push_call_argument_packed(obj.position(), obj.op(), args);
    push_root_call(obj.position(), obj.op(), args);
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
        const c4::ffm::symbol& symbol;
    };
}

std::vector<c4::ffm::block_argument*>
c4::ffm_mapper::make_argument_list(const ast2::block_args* args) {
    std::vector<ffm::block_argument*> result;
    if (_closure)
        result.emplace_back(_ffm_context.build_block_argument(
            position::pseudo_position(), "@ctx", 0));
    if (!args) return result;

    result.reserve(result.size() + args->block_arguments().size());

    std::ranges::transform(
        args->block_arguments(),
        std::back_inserter(result),
        [this](const auto& arg) {
            auto ret = _ffm_context.build_block_argument(arg.position(),
                                                         arg.name(),
                                                         arg.base_arity());
            arg.template emplace_attribute<argument_attribute>("argument", ret);
            return ret;
        });

    return result;
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

c4::ffm::function_declaration*
c4::ffm_mapper::declare_function(const ffm::symbol& sym, std::vector<ffm::block_argument*>&& args) {
    if (const auto fn = find_function_declaration(sym)) return fn;

    const auto declaration = _ffm_context.build_function_declaration(sym, _closure, true, std::move(args));
    const auto fun = _ffm_context.build_function(declaration);
    _roots.push_back(fun);
    return declaration;
}

c4::ffm::function_declaration*
c4::ffm_mapper::declare_extern_function(const ffm::symbol& sym) {
    if (const auto fn = find_function_declaration(sym)) return fn;

    const auto declaration = _ffm_context.build_function_declaration(sym, _closure, false, {});
    const auto fun = _ffm_context.build_function(declaration);
    _roots.push_back(fun);
    return declaration;
}

c4::recursive_scope<c4::ffm::function_call*>
c4::ffm_mapper::enter_call_arguments(ffm::function_call* call) {
    return recursive_scope(_current_call, call);
}

c4::recursive_scope<c4::ffm::function_pack*>
c4::ffm_mapper::enter_pack_arguments(ffm::function_pack* pack) {
    return recursive_scope(_current_pack, pack);
}

c4::recursive_scope<c4::ffm::function_definition*>
c4::ffm_mapper::define_function(const ffm::function_declaration* decl) {
    const auto definition = _ffm_context.build_function_definition(decl);
    const auto fn = _ffm_context.build_function(definition);
    _roots.push_back(fn);
    return recursive_scope(_current_function, definition);
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
    return fmt::format("{}#{}/{}",
                       1 + numeric_length(id) + 1 + numeric_length(arity),
                       id,
                       arity);
}
