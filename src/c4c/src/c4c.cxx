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
 * src/c4c/src/c4c --
 *   
 */

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX

#include <fstream>
#include <ranges>
#include <unordered_set>

#include <c4/ast_dumper.hxx>

#include <c4/p2/parser.hxx>
#include <c4/p2/lex/lexer.hxx>
#include <c4c/c4_runtime_emitter.hxx>

#include <c4c/source_file.hxx>
#include <c4rt/datum.h>
#include <libassert/assert.hpp>

#include <llvm/ADT/APFloat.h>
#include <llvm/ADT/STLExtras.h>
#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/Constants.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Type.h>
#include <llvm/IR/Verifier.h>
#include <llvm/MC/TargetRegistry.h>
#include <llvm/Support/FileSystem.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/Target/TargetMachine.h>
#include <llvm/Target/TargetOptions.h>
#include <llvm/TargetParser/Host.h>
#include <llvm/IR/LegacyPassManager.h>

#include <lyra/lyra.hpp>
#include <utility>

#ifdef _WIN32
#  include <windows.h>
#endif

using namespace std::literals;

struct ostream_deleter {
    void
    operator()(const std::ostream* os) const {
        if (os == &std::cout) return;
        delete os;
    }
};

using outstream_ptr = std::unique_ptr<std::ostream, ostream_deleter>;

outstream_ptr
open_outstream(const std::filesystem::path& path) {
    if (path == "-") return outstream_ptr(&std::cout);

    auto ptr = outstream_ptr(new std::ofstream(path));
    if (!*ptr) throw std::runtime_error("could not open output file: " + path.string());

    return ptr;
}

template<class It, class S = It>
void
dump_ast(It begin, S end, std::ostream& out) {
    c4::ast_dumper dumper(out);
    std::for_each(std::move(begin), std::move(end),
                  [&dumper, &out](const auto& expr) {
                      expr->accept(dumper);
                      out << "\n";
                  });
}

void
initialize_targets();

struct llvm_value_attribute final : c4::ast2::tags::typed_attribute<llvm::Value*> {
    explicit
    llvm_value_attribute(llvm::Value* const& value)
        : typed_attribute{value} { }
};

struct global_constant_emitter final : c4::ast2::visitor<
            c4::ast2::float_literal,
            c4::ast2::integer_literal,
            c4::ast2::string_literal> {
    global_constant_emitter(llvm::LLVMContext& context,
                            llvm::Module& module,
                            llvm::IRBuilder<>& builder)
        : context(context)
        , module(module)
        , builder(builder) { }

    llvm::Value*
    get_loaded_global(const llvm::Twine& name) {
        ASSERT(value, "global_constant_emitter needs to visit the value before it can generate the load to it");
        value->setName(name.concat(_value_type_suffix));
        return builder.CreateLoad(llvm::Type::getInt64Ty(context), value);
    }

    void
    do_visit(const c4::ast2::float_literal& obj) override {
        const auto rt_value = c4rt_datum_from_double(obj.value());
        _value_type_suffix = "_fl";
        create_global(rt_value);
    }

    void
    do_visit(const c4::ast2::integer_literal& obj) override {
        ASSERT(obj.value() < std::numeric_limits<int32_t>::max());
        _value_type_suffix = "_il";
        const auto rt_value = c4rt_datum_from_int32(static_cast<int32_t>(obj.value()));
        create_global(rt_value);
        obj.emplace_attribute<llvm_value_attribute>("value", value);
    }

    void
    do_visit(const c4::ast2::string_literal& obj) override {
        ASSERT(obj.value().size() < 6); // todo name SSO limit
        _value_type_suffix = "_ssl";
        const auto rt_value = c4rt_datum_from_string_sz(obj.value().data(), obj.value().size());
        create_global(rt_value);
    }

    llvm::Value* value{};
    llvm::LLVMContext& context;
    llvm::Module& module;
    llvm::IRBuilder<>& builder;

private:
    std::string_view _value_type_suffix;

    void
    create_global(const c4_datum_t datum) {
        const auto val = new llvm::GlobalVariable(llvm::Type::getInt64Ty(context), true,
                                                  llvm::GlobalValue::PrivateLinkage,
                                                  llvm::ConstantInt::get(llvm::Type::getInt64Ty(context), datum));
        value = val;
        module.insertGlobalVariable(val);
    }
};

struct ir_emitter final : c4::ast2::visitor<
            c4::ast2::expression,
            c4::ast2::block,
            c4::ast2::block_args,
            c4::ast2::let_expression,
            c4::ast2::float_literal,
            c4::ast2::integer_literal,
            c4::ast2::string_literal,
            c4::ast2::fn_call,
            c4::ast2::unary_op_call,
            c4::ast2::binary_op_call
        > {
    struct last_value {
        llvm::Value* value{};
        bool constant{};

        void
        set_expr(llvm::Value* value) {
            this->value = value;
            constant = false;
        }

        void
        set_constant(llvm::Value* value) {
            this->value = value;
            constant = true;
        }
    };

    ir_emitter(c4c::c4_runtime_emitter& rt_emitter,
               c4::ast2::ast_context& ast_context,
               llvm::LLVMContext& context,
               llvm::Module& module,
               llvm::IRBuilder<>& builder,
               std::vector<c4::ast2::undef_symbol> const& promised_symbols)
        : rt_emitter{rt_emitter}
        , ast_context{ast_context}
        , context{context}
        , module{module}
        , builder{builder}
        , promised_symbols(promised_symbols.begin(), promised_symbols.end()) {
        const auto entry_type = llvm::FunctionType::get(llvm::Type::getInt64Ty(context), {}, false);
        entry = llvm::Function::Create(entry_type, llvm::Function::ExternalLinkage, "_c4__entry", module);

        const auto main_body = llvm::BasicBlock::Create(context, "body", entry);
        builder.SetInsertPoint(main_body);
    }

    void
    do_visit(const c4::ast2::expression& obj) override {
        obj.accept_skip_self(*this);
    }

    void
    do_visit(const c4::ast2::fn_call& obj) override {
        const auto callee = lookup(obj.sym());
        ASSERT(callee, "called function cannot be found",
               obj.sym().name(),
               promised_symbols,
               _loaded_promised_symbols);

        if (callee->getType()->isIntegerTy()) {
            last.set_expr(callee);
            return;
        }

        // has arguments, so callee must be a function type
        ASSERT(callee->getType()->isPointerTy(),
               "fn called with parameters does not have function (pointer) type",
               callee->getName(),
               callee->getType()->getTypeID());
        const auto fn = cast<llvm::Function>(callee);
        ASSERT(fn);

        if (const auto fn_type = fn->getFunctionType();
            fn_type->getNumParams() > 0
            && fn_type->getParamType(0)->isPointerTy()) {
            const auto ref = obj.sym().references();
            const auto context_ref = ref->attribute_value<llvm::Value*>("context");
            ASSERT(context_ref, "referenced symbol must have a context attribute (0th parameter type is ptr)",
                   obj.sym().mangle(),
                   context_ref);
            const auto context = *context_ref;

            std::vector<llvm::Value*> args(obj.args().size() + 1);
            args[0] = context;
            std::transform(obj.args().begin(), obj.args().end(),
                           next(args.begin()),
                           [this](const c4::ast2::expression* const& arg) {
                               arg->accept(*this);
                               return this->last.value;
                           });

            const auto call = builder.CreateCall(fn_type, callee, args);
            last.set_expr(call);

            return;
        }

        std::vector<llvm::Value*> args(obj.args().size());
        std::transform(obj.args().begin(), obj.args().end(), args.begin(),
                       [this](const c4::ast2::expression* const& arg) {
                           arg->accept(*this);
                           return this->last.value;
                       });

        const auto call = builder.CreateCall(fn->getFunctionType(), callee, args);
        last.set_expr(call);
    }

    void
    do_visit(const c4::ast2::unary_op_call& obj) override {
        const auto callee_val = lookup_symbol(obj.op().mangle());
        ASSERT(callee_val,
               "callee symbol must be known at the point of call",
               obj.op().name(),
               obj.op().mangle(),
               promised_symbols);

        if (callee_val->getType()->isIntegerTy()) {
            last.set_expr(callee_val);
            return;
        }

        // has arguments, so callee must be a function type
        ASSERT(callee_val->getType()->isPointerTy(),
               "fn called with parameters does not have function (pointer) type",
               callee_val->getName(),
               callee_val->getType()->getTypeID());
        const auto fn = cast<llvm::Function>(callee_val);

        std::vector<llvm::Value*> args(1);
        std::ranges::transform(std::array{&obj.operand()}, args.begin(), [this](const c4::ast2::expression* arg) {
            arg->accept(*this);
            return this->last.value;
        });

        const auto call = builder.CreateCall(fn->getFunctionType(), callee_val, args);
        last.set_expr(call);
    }

    void
    do_visit(const c4::ast2::binary_op_call& obj) override {
        const auto callee_val = lookup_symbol(obj.op().mangle());
        ASSERT(callee_val,
               "callee symbol must be known at the point of call",
               obj.op().name(),
               obj.op().mangle(),
               promised_symbols);

        if (callee_val->getType()->isIntegerTy()) {
            last.set_expr(callee_val);
            return;
        }

        // has arguments, so callee must be a function type
        ASSERT(callee_val->getType()->isPointerTy(),
               "fn called with parameters does not have function (pointer) type",
               callee_val->getName(),
               callee_val->getType()->getTypeID());
        const auto fn = cast<llvm::Function>(callee_val);

        std::vector<llvm::Value*> args(2);
        std::ranges::transform(std::array{&obj.left(), &obj.right()}, args.begin(),
                               [this](const c4::ast2::expression* arg) {
                                   arg->accept(*this);
                                   return this->last.value;
                               });

        const auto call = builder.CreateCall(fn->getFunctionType(), callee_val, args);
        last.set_expr(call);
    }

    void
    do_visit(const c4::ast2::block_args& obj) override {
        const auto formal_args_size = _function_is_closure
                                      ? _active_function->arg_size() - 1
                                      : _active_function->arg_size();
        ASSERT(formal_args_size == obj.size(),
               "defining function with invalid argument size block");

        const auto args = _active_function->args();
        auto arg_begin = args.begin();
        std::size_t i = 0;
        if (_function_is_closure) {
            ++i;
            std::advance(arg_begin, 1);
        }

        for (const auto& [formal_arg, ll_arg] : std::views::zip(obj.args(),
                                                                std::span{arg_begin, args.end()})) {
            ASSERT(ll_arg.getType() == llvm::Type::getInt64Ty(context),
                   "trying to name non datum_t type",
                   ll_arg.getType()->getTypeID(),
                   formal_arg->name(),
                   _active_function->getFunctionType()->params(),
                   _active_function->getName());
            ll_arg.setName(formal_arg->mangle());
            obj.argument_reference(i++).emplace_attribute<llvm_value_attribute>("value", &ll_arg);
        }
    }

    void
    do_visit(const c4::ast2::block& obj) override {
        if (obj.args())
            obj.args()->accept(*this);
        if (obj.requires_context()) {
            // if name is not empty, the ctx contained only globally known things
            // and is not actually the context parameter
            if (_active_function->arg_size() > 0
                && _active_function->arg_begin()->getName().empty())
                _active_function->arg_begin()->setName("ctx");
        }
        emit_fn_body_from_block(obj);
    }

    void
    do_visit(const c4::ast2::let_expression& obj) override {
        const auto obj_name = obj.mangled_name();
        const auto arity = obj.symbol_arity();
        const auto memory_len = _block_name.size();
        if (_block_name.empty()) {
            _block_name = obj_name;
        }
        else {
            _block_name = fmt::format("{}${}", _block_name, obj_name);
        }

        if (obj.value().const_evaluable()) {
            global_constant_emitter constant_emitter(context, module, builder);
            obj.value().accept_skip_self(constant_emitter);
            last.set_constant(constant_emitter.get_loaded_global(_block_name));
            obj.emplace_attribute<llvm_value_attribute>("value", last.value);
            // _known_symbols[_block_name] = last.value;
            _block_name = _block_name.substr(0, memory_len);
            return;
        }

        // WARNING: HORRID HACK: USING WHILE AS IF TO ALLOW BREAKING IT IN THE
        //   MIDDLE, REFACTOR LOGIC INTO FN
        bool effective_closure = obj.value().closure();
        while (effective_closure) {
            const auto datum_t = llvm::Type::getInt64Ty(context);
            const auto index_t = llvm::Type::getInt64Ty(context);

            std::vector<const c4::ast2::symbol*> effective_closure_symbols;
            effective_closure_symbols.reserve(obj.value().closure_symbols().size());
            std::ranges::transform(obj.value().closure_symbols(),
                                   std::back_inserter(effective_closure_symbols),
                                   [](const auto& sym) { return &sym; });
            std::erase_if(effective_closure_symbols,
                          [this](const auto& sym) mutable { return skip_in_context(*sym); });
            if (effective_closure_symbols.empty()) {
                effective_closure = false;
                break;
            }

            const auto call_ctx =
                    builder.CreateAlloca(
                        datum_t,
                        llvm::ConstantInt::get(datum_t, effective_closure_symbols.size()),
                        {_block_name, ".ctx"});

            obj.emplace_attribute<llvm_value_attribute>("context", call_ctx);

            size_t idx = 0;
            for (const auto& closure_symbol : effective_closure_symbols) {
                const auto store_ptr = builder.CreateGEP(
                    datum_t,
                    call_ctx,
                    llvm::ConstantInt::get(index_t, idx++),
                    llvm::Twine(_block_name, ".ctx.").concat({closure_symbol->name(), ".addr"})
                );

                ASSERT(closure_symbol->references(),
                       "symbol captured is not emitted",
                       closure_symbol->name(),
                       closure_symbol->arity());
                const auto attr = closure_symbol->references()->get_attribute("value");
                ASSERT(attr,
                       "symbol captured is not emitted (refers to something without value attribute)",
                       closure_symbol->name(),
                       closure_symbol->arity(),
                       closure_symbol->references());
                const auto val = attr->value<llvm::Value*>();
                DEBUG_ASSERT(val,
                             "symbol's value does not hold llvm::Value*",
                             val);
                builder.CreateStore(*val, store_ptr);
            }
            break;
        }

        const auto fn_type = rt_emitter.get_c4_funtype(arity, effective_closure);
        const auto fn = llvm::Function::Create(fn_type, llvm::GlobalValue::ExternalLinkage,
                                               {"_C", _block_name},
                                               module);
        obj.emplace_attribute<llvm_value_attribute>("value", fn);

        enter_function_emission(fn, effective_closure, obj.value(), [this](const auto& val) {
            val.accept(*this);
            build_return();
        });

        _block_name = _block_name.substr(0, memory_len);
    }

    void
    do_visit(const c4::ast2::float_literal& obj) override {
        const auto val = c4rt_datum_from_double(obj.value());
        last.set_constant(llvm::ConstantInt::get(llvm::Type::getInt64Ty(context), val));
        obj.emplace_attribute<llvm_value_attribute>("value", last.value);
    }

    void
    do_visit(const c4::ast2::string_literal& obj) override {
        if (obj.const_evaluable()) {
            global_constant_emitter constant_emitter(context, module, builder);
            obj.accept(constant_emitter);
            last.set_constant(constant_emitter.get_loaded_global(_block_name));
            obj.emplace_attribute<llvm_value_attribute>("value", last.value);
            return;
        }
        const auto global = builder.CreateGlobalStringPtr(obj.value(), {_block_name, "_sl"}, 0, &module);
        const auto value = rt_emitter.emit_rt_call(c4c::c4rt_symbol::DatumFromString, builder, global);
        last.set_expr(value);
        obj.emplace_attribute<llvm_value_attribute>("value", last.value);
        _need_cleanup.push_back(value);
    }

    void
    do_visit(const c4::ast2::integer_literal& obj) override {
        if (const auto val = obj.value();
            val < static_cast<std::int64_t>(std::numeric_limits<int32_t>::max())) {
            const auto ret = c4rt_datum_from_int32(static_cast<int32_t>(obj.value()));
            last.set_constant(llvm::ConstantInt::get(llvm::Type::getInt64Ty(context), ret));
            obj.emplace_attribute<llvm_value_attribute>("value", last.value);
        }
        else {
            const auto value = rt_emitter.emit_rt_call(c4c::c4rt_symbol::DatumFromInt64, builder,
                                                       llvm::ConstantInt::get(llvm::Type::getInt64Ty(context), val));
            last.set_expr(value);
            obj.emplace_attribute<llvm_value_attribute>("value", last.value);
            _need_cleanup.push_back(value);
        }
    }

    void
    finalize() {
        _finalized = true;
        build_return();
        generate_cleanup(&entry->back(), _need_cleanup);
    }

    ~ir_emitter() noexcept override {
        ASSERT(_finalized, "ir_emitter must be finalized: call finalize on it before it dies");
    }

    llvm::Function* entry;
    last_value last{};
    c4c::c4_runtime_emitter& rt_emitter;
    c4::ast2::ast_context& ast_context;
    llvm::LLVMContext& context;
    llvm::Module& module;
    llvm::IRBuilder<>& builder;

private:
    bool
    skip_in_context(const c4::ast2::symbol& sym) {
        const auto fn = lookup(sym);
        if (fn == nullptr) return true;
        return isa<llvm::Function>(fn);
    }

    llvm::Value*
    try_materialize_promise(const std::string_view sym) {
        const auto promised_sym_it = std::ranges::find_if(promised_symbols, [sym](const auto undef_sym) {
            return undef_sym.mangle() == sym;
        });
        if (promised_sym_it == promised_symbols.end())
            return nullptr;

        const auto promised_sym = *promised_sym_it;
        promised_symbols.erase(promised_sym_it);

        const auto promised_fn_type = rt_emitter.get_c4_funtype(promised_sym.arity());
        const auto promised_fn = llvm::Function::Create(promised_fn_type, llvm::Function::ExternalLinkage,
                                                        {"_C", promised_sym.mangle()},
                                                        module);

        _loaded_promised_symbols[std::string(sym)] = promised_fn;

        return promised_fn;
    }

    llvm::Value*
    lookup(const c4::ast2::symbol& sym) {
        if (const auto ref = sym.references()) {
            const auto callee_ref = ref->attribute_value<llvm::Value*>("value");
            ASSERT(callee_ref, "referenced symbol must have a value attribute to the callee",
                   sym.mangle(),
                   callee_ref);
            return *callee_ref;
        }

        if (const auto known_it = _loaded_promised_symbols.find(sym.mangle());
            known_it != _loaded_promised_symbols.end()) {
            return known_it->second;
        }

        return try_materialize_promise(sym.mangle());
    }

    llvm::Value*
    lookup_symbol(const std::string_view sym) {
        if (const auto known_it = _loaded_promised_symbols.find(std::string(sym));
            known_it != _loaded_promised_symbols.end()) {
            return known_it->second;
        }

        return try_materialize_promise(sym);
    }

    void
    build_return() {
        if (const auto it = std::ranges::find(_need_cleanup, last.value);
            it != _need_cleanup.end()) {
            // don't clean up stuff we are returning
            _need_cleanup.erase(it);
        }
        builder.CreateRet(last.value);
    }

    struct scope_override_fixer {
        void
        add_symbol(c4::ast2::tags::referable* sym, llvm::Value* new_value) {
            auto old = sym->emplace_attribute<llvm_value_attribute>("value", new_value);
            _overridden.emplace_back(sym, std::move(old));
        }

        void
        operator()();

    private:
        struct scope_override {
            c4::ast2::tags::referable* symbol;
            std::unique_ptr<c4::ast2::tags::attribute> old;
        };

        std::vector<scope_override> _overridden{};
    };

    void
    emit_fn_body_from_block(const c4::ast2::block& obj) {
        scope_override_fixer fixer;
        if (obj.requires_context() && _active_function->arg_size() > 0) {
            emit_context_expansion(fixer, obj);
        }
        for (const auto& expr : obj.expressions()) {
            expr->accept_skip_self(*this);
        }
        fixer();
    }

    void
    emit_context_expansion(scope_override_fixer& fixer, const c4::ast2::block& obj) {
        const auto body_bb = builder.GetInsertBlock();
        const auto ip = builder.saveIP();

        const auto block = llvm::BasicBlock::Create(context, "ctx_exp", _active_function, body_bb);
        builder.SetInsertPoint(block);

        const auto datum_t = llvm::Type::getInt64Ty(context);
        const auto gep_index_t = llvm::Type::getInt64Ty(context);
        const auto ctx_obj = _active_function->getArg(0);
        if (!ctx_obj->getType()->isPointerTy()) {
            builder.CreateBr(body_bb);
            builder.restoreIP(ip);
            return;
        }

        std::unordered_set<std::string_view> expanded{};
        for (size_t idx = 0;
             auto& sym : obj.effective_context_symbols()) {
            if (skip_in_context(sym)) continue;

            const auto ctx_param_ptr = builder.CreateGEP(datum_t, ctx_obj,
                                                         llvm::ConstantInt::get(gep_index_t, idx++),
                                                         {sym.name(), ".addr"});
            const auto ctx_param = builder.CreateLoad(datum_t, ctx_param_ptr, sym.name());
            if (const auto ref = sym.references()) fixer.add_symbol(ref, ctx_param);
        }

        builder.CreateBr(body_bb);
        builder.restoreIP(ip);
    }

    void
    generate_cleanup(llvm::BasicBlock* fn_body,
                     const std::span<llvm::Value*const> cleanup) const {
        if (cleanup.empty()) return;

        const auto block_sz = static_cast<long long>(fn_body->size());
        const auto cleanup_block = fn_body->splitBasicBlock(std::next(fn_body->begin(), block_sz - 1), "cleanup");
        builder.SetInsertPoint(cleanup_block->begin());
        for (const auto& to_clean : cleanup) {
            rt_emitter.emit_rt_call(c4c::c4rt_symbol::DatumFree, builder, to_clean);
        }
    }

    template<class Fn>
    void
    enter_function_emission(llvm::Function* fn,
                            const bool closure,
                            const c4::ast2::expression& val,
                            Fn&& emitter) {
        const auto last_ip = builder.saveIP();
        const auto fn_mem = _active_function;
        const auto closure_mem = _function_is_closure;
        const auto cleanup_mem = std::exchange(_need_cleanup, {});

        _active_function = fn;
        _function_is_closure = closure;

        const auto fn_body = llvm::BasicBlock::Create(context, "body", fn);
        builder.SetInsertPoint(fn_body);
        std::invoke(std::forward<Fn>(emitter), val);

        generate_cleanup(fn_body, std::exchange(_need_cleanup, cleanup_mem));
        _function_is_closure = closure_mem;
        _active_function = fn_mem;
        builder.restoreIP(last_ip);
    }

    bool _finalized{false};
    bool _function_is_closure{false};
    std::string _block_name{};
    llvm::Function* _active_function{};
    std::vector<llvm::Value*> _need_cleanup{};

    std::unordered_map<std::string, llvm::Value*> _loaded_promised_symbols;
    std::unordered_map<std::string, std::stack<llvm::Value*>> _closure_symbols;
    std::vector<c4::ast2::undef_symbol> promised_symbols;
};

int
main(int argc, const char** argv) {
    std::filesystem::path out_path;
    std::filesystem::path src_path;
    std::string target_arch;
    std::string dump_type;
    bool show_help = false;

    const auto cli = lyra::cli()
                     | lyra::help(show_help).description(
                         "Compile a C4 script into an object file.")(
                         "Do not compile, print help and exit.")
                     | lyra::opt(out_path, "output")["-o"]["--output"](
                         "The name of the output file. When -d is set, STDOUT if `-'.")
                     | lyra::opt(dump_type, "dump")["-d"]["--dump"](
                         "Do not compile, dump code instead. [AST, IR, ASM]").choices("AST", "IR", "ASM")
                     | lyra::arg(src_path, "source")(
                         "The C4 source file to compile.").required()
                     | lyra::opt(target_arch, "target arch triplet")["-T"]["--target"](
                         "The target triplet to produce the binary for.");

    if (const auto result = cli.parse({argc, argv});
        !result) {
        std::cerr << "fatal: " << result.message() << "\n";
        std::cerr << cli << std::endl;
        return 1;
    }

    if (show_help) {
        std::cout << cli << std::endl;
        return 1;
    }

    c4c::source_file src(src_path);
    if (out_path.empty()) {
        out_path = src_path;
        out_path.replace_extension(".o");
    }

    src_path = absolute(src_path);
    c4::ast2::ast_context ast_context;
    c4::p2::lexer lexer(src_path.string(), src.begin(), src.end());
    c4::p2::parser parser(ast_context, std::move(lexer));

    parser.declare_binop("+", 4, false);
    parser.declare_binop("-", 4, false);
    parser.declare_binop("*", 5, false);
    parser.declare_binop("/", 5, false);
    parser.declare_binop("^", 5, true);
    parser.declare_binop("==", 3, true);

    parser.declare_binop("<<", 6, false);
    parser.declare_binop(">>", 5, true);

    parser.declare_uniop("~");

    parser.declare_symbol("print", 1, nullptr);
    parser.declare_symbol("println", 1, nullptr);
    parser.declare_symbol("add", 2, nullptr);
    parser.declare_symbol("if", 3, nullptr);
    parser.declare_symbol("str_empty", 1, nullptr);
    parser.declare_symbol("cat", 2, nullptr);
    parser.declare_symbol("readln", 0, nullptr);

    try {
        const auto script = parser.parse_script();
        if (!parser.valid())
            return 1;

        if (dump_type == "AST") {
            auto outstrm = open_outstream(out_path);
            dump_ast(script.begin(), script.end(), *outstrm);
            return 0;
        }

        initialize_targets();

        llvm::LLVMContext context;
        llvm::Module module(src_path.string(), context);
        llvm::IRBuilder<> builder(context);

        const auto target_triple = target_arch.empty()
                                   ? llvm::sys::getDefaultTargetTriple()
                                   : target_arch;
        std::string target_error;
        const auto target = llvm::TargetRegistry::lookupTarget(target_triple, target_error);
        if (!target) {
            std::cerr << "fatal: " << target_error << "\n";
            return 2;
        }

        const auto machine = target->createTargetMachine(target_triple, "generic", "", {}, llvm::Reloc::PIC_);
        module.setDataLayout(machine->createDataLayout());
        module.setTargetTriple(target_triple);

        c4c::c4_runtime_emitter rt_emitter(context, module);
        auto ir = ir_emitter(rt_emitter, ast_context, context, module, builder, parser.promised_symbols());
        for (const auto& expression : script) {
            expression->accept(ir);
        }
        ir.finalize();

        auto entry_type = llvm::FunctionType::get(llvm::Type::getInt32Ty(context), false);
        auto crt = llvm::Function::Create(entry_type, llvm::Function::ExternalLinkage,
                                          "mainCRTStartup",
                                          module);
        auto crt_bb = llvm::BasicBlock::Create(context, "", crt);
        builder.SetInsertPoint(crt_bb);

        const auto c4_ret = builder.CreateCall(ir.entry);
        const auto main_ret_call = rt_emitter.emit_rt_call(c4c::c4rt_symbol::DatumCoerceInt32, builder,
                                                           {c4_ret});
        rt_emitter.emit_rt_call(c4c::c4rt_symbol::DatumFree, builder, {c4_ret});
        builder.CreateRet(main_ret_call);

        if (dump_type == "IR") {
            std::string dump;
            llvm::raw_string_ostream os(dump);
            module.print(os, nullptr);
            *open_outstream(out_path) << dump;
            return 0;
        }

        if (out_path == "-" && dump_type != "ASM") {
            std::cerr << "fatal: cowardly refusing to dump binary data to STDOUT\n";
            return 1;
        }

        std::error_code ec;
        llvm::raw_fd_ostream fout(out_path.string(), ec, llvm::sys::fs::OF_None);
        if (ec) {
            std::cerr << "fatal: could not open file `" << out_path << "'\n";
            return 1;
        }

        auto out_type = llvm::CodeGenFileType::ObjectFile;
        if (dump_type == "ASM")
            out_type = llvm::CodeGenFileType::AssemblyFile;

        llvm::legacy::PassManager pass_mgr;
        machine->addPassesToEmitFile(pass_mgr,
                                     fout,
                                     nullptr,
                                     out_type);
        pass_mgr.run(module);
        fout.flush();
    }
    catch (const std::exception& e) {
        std::cerr << "fatal: " << e.what() << "\n";
        return 1;
    }
}

void
initialize_targets() {
    LLVMInitializeX86Target();
    LLVMInitializeX86TargetInfo();
    LLVMInitializeX86TargetMC();
    LLVMInitializeX86AsmPrinter();

    LLVMInitializeAArch64Target();
    LLVMInitializeAArch64TargetInfo();
    LLVMInitializeAArch64TargetMC();
    LLVMInitializeAArch64AsmPrinter();

    LLVMInitializeRISCVTarget();
    LLVMInitializeRISCVTargetInfo();
    LLVMInitializeRISCVTargetInfo();
    LLVMInitializeRISCVTargetMC();
    LLVMInitializeRISCVAsmPrinter();

    LLVMInitializeSparcTarget();
    LLVMInitializeSparcTargetInfo();
    LLVMInitializeSparcTargetMC();
    LLVMInitializeSparcAsmPrinter();

    LLVMInitializeSystemZTarget();
    LLVMInitializeSystemZTargetInfo();
    LLVMInitializeSystemZTargetMC();
    LLVMInitializeSystemZAsmPrinter();
}

void
ir_emitter::scope_override_fixer::operator()() {
    std::for_each(_overridden.rbegin(), _overridden.rend(), [](const scope_override& fixee) {
        fixee.symbol->emplace_attribute<llvm_value_attribute>("value", *fixee.old.get()->value<llvm::Value*>());
    });
}
