# blAST project
#
# Copyright (c) 2025 András Bodor <bodand@proton.me>
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
#
# - Redistributions of source code must retain the above copyright notice, this
#   list of conditions and the following disclaimer.
#
# - Redistributions in binary form must reproduce the above copyright notice,
#   this list of conditions and the following disclaimer in the documentation
#   and/or other materials provided with the distribution.
#
# - Neither the name of the copyright holder nor the names of its contributors
#   may be used to endorse or promote products derived from this software
#   without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
# AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
# DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
# FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
# DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
# SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
# CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
# OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#
# Originally created: 2025-08-21.
#
# src/c4rt2/CMakeLists.txt --
#   c4rt2 provides two interfaces: c4rt2 and c4rt2c. Both of them provide
#   aim at providing the runtime functionality required by C4, but c4rt2
#   provides this using a stable ABI interface in C. This can be linked against
#   using LLVM's JIT runtime or a normal linker when using c4c while c4rt2c
#   provides a subset of this functionality in an unstable C++ API meant to only
#   be used by c4c and c4 when generating LLVM IR.
#   Most code is ported over from c4rt.

set(rt_bridge_llvm
    src/c4rt3/c4int_datum_true.ll
    src/c4rt3/c4int_datum_false.ll
    )
set(rt_bridge_obj "")

foreach (src IN LISTS rt_bridge_llvm)
    cmake_path(GET src FILENAME srcbase)
    set(obj "${CMAKE_CURRENT_BINARY_DIR}/${srcbase}.o")

    add_custom_command(
            OUTPUT ${obj}
            COMMAND ${LLVM_TOOLS_BINARY_DIR}/llc
            -filetype=obj
            -relocation-model=pic
            -O3
            ${CMAKE_CURRENT_SOURCE_DIR}/${src}
            -o ${obj}
            DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/${src}
            COMMENT "Compiling LLVM IR ${srcbase}.ll"
            )

    list(APPEND rt_bridge_obj ${obj})
endforeach ()

add_library(c4rt3 STATIC
            include/c4rt3/c4rt.h
            src/c4rt3/c4_datum_type_of.c
            src/c4rt3/c4_datum_is_nil.c
            src/c4rt3/c4_datum_get_int64.c
            src/c4rt3/c4_datum_get_double.c
            src/c4rt3/c4_datum_get_string.c
            src/c4rt3/c4_datum_coerce_int64.c
            src/c4rt3/c4_datum_coerce_double.c
            src/c4rt3/c4_datum_coerce_string.c
            src/c4rt3/c4_datum_from_boolean.c
            src/c4rt3/c4_datum_from_string_sz.c
            src/c4rt3/c4_datum_from_int64.c
            src/c4rt3/c4_datum_from_double.c
            src/c4rt3/c4_datum_from_nil.c

            src/c4rt3/internal_type.h
            ${rt_bridge_obj}

            src/c4rt3/ska/fmt.h
            src/c4rt3/ska/int64_fmt_generic.c
            src/c4rt3/ska/uint64_fmt_generic.c
            src/c4rt3/c4_datum_cmp.c
            )
target_include_directories(c4rt3
                           PUBLIC $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
                           PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
                           PRIVATE $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>)
target_link_libraries(c4rt3 PUBLIC BDWgc::gc RYU::ryu)

add_library(c4rt2c STATIC
            include/c4rt2c/llvm_value_attribute.hxx
            src/c4rt2c/llvm_value_attribute.cxx

            include/c4rt2c/source_file.hxx
            src/c4rt2c/source_file.cxx

            include/c4rt2c/ast2_ir_emitter.hxx
            src/c4rt2c/ast2_ir_emitter/init.cxx
            src/c4rt2c/ast2_ir_emitter/active_expression.cxx
            src/c4rt2c/ast2_ir_emitter/allocate_argv.cxx
            src/c4rt2c/ast2_ir_emitter/ast2_ir_emitter.cxx
            src/c4rt2c/ast2_ir_emitter/declare_function.cxx
            src/c4rt2c/ast2_ir_emitter/declare_symbols.cxx
            src/c4rt2c/ast2_ir_emitter/define.cxx
            src/c4rt2c/ast2_ir_emitter/define_function.cxx
            src/c4rt2c/ast2_ir_emitter/define_global_var.cxx
            src/c4rt2c/ast2_ir_emitter/define_variable.cxx
            src/c4rt2c/ast2_ir_emitter/do_visit_binary_op_call.cxx
            src/c4rt2c/ast2_ir_emitter/do_visit_block.cxx
            src/c4rt2c/ast2_ir_emitter/do_visit_dynamic_call.cxx
            src/c4rt2c/ast2_ir_emitter/do_visit_expression.cxx
            src/c4rt2c/ast2_ir_emitter/do_visit_float_literal.cxx
            src/c4rt2c/ast2_ir_emitter/do_visit_fn_call.cxx
            src/c4rt2c/ast2_ir_emitter/do_visit_integer_literal.cxx
            src/c4rt2c/ast2_ir_emitter/do_visit_let_expression.cxx
            src/c4rt2c/ast2_ir_emitter/do_visit_string_literal.cxx
            src/c4rt2c/ast2_ir_emitter/do_visit_unary_op_call.cxx
            src/c4rt2c/ast2_ir_emitter/emit_function_call.cxx
            src/c4rt2c/ast2_ir_emitter/emit_named_function.cxx
            src/c4rt2c/ast2_ir_emitter/finalize.cxx
            src/c4rt2c/ast2_ir_emitter/thunked_symbol.cxx

            src/c4rt2c/ast2_ir_emitter/function_scope/continue_at.cxx
            src/c4rt2c/ast2_ir_emitter/function_scope/definition.cxx
            src/c4rt2c/ast2_ir_emitter/function_scope/function_scope.cxx
            src/c4rt2c/ast2_ir_emitter/function_scope/start_function.cxx

            src/c4rt2c/ast2_ir_emitter/name_manager/format_symbol_stack.cxx
            src/c4rt2c/ast2_ir_emitter/name_manager/global_name.cxx
            src/c4rt2c/ast2_ir_emitter/name_manager/lambda_name.cxx
            src/c4rt2c/ast2_ir_emitter/name_manager/mangle_symbol.cxx
            src/c4rt2c/ast2_ir_emitter/name_manager/mangle_symbol_stack.cxx
            src/c4rt2c/ast2_ir_emitter/name_manager/string_name.cxx

            include/c4rt2c/runtime_fn.hxx
            src/c4rt2c/runtime_fn/arg.cxx
            src/c4rt2c/runtime_fn/define.cxx
            src/c4rt2c/runtime_fn/file_scoped.cxx
            src/c4rt2c/runtime_fn/~runtime_fn_debug_info_builder.cxx

            include/c4rt2c/scoped_scope.hxx

            include/c4rt2c/runtime_emitter.hxx
            src/c4rt2c/runtime_emitter/allocate.cxx
            src/c4rt2c/runtime_emitter/allocate_array.cxx
            src/c4rt2c/runtime_emitter/complete_thunk.cxx
            src/c4rt2c/runtime_emitter/evaluate.cxx
            src/c4rt2c/runtime_emitter/get_completion_K.cxx
            src/c4rt2c/runtime_emitter/get_completion_self.cxx
            src/c4rt2c/runtime_emitter/get_completion_thunk.cxx
            src/c4rt2c/runtime_emitter/get_datum_argv.cxx
            src/c4rt2c/runtime_emitter/get_datum_argv_sz.cxx
            src/c4rt2c/runtime_emitter/get_datum_type.cxx
            src/c4rt2c/runtime_emitter/get_datum_value.cxx
            src/c4rt2c/runtime_emitter/make_apply_thunk.cxx
            src/c4rt2c/runtime_emitter/make_datum_block.cxx
            src/c4rt2c/runtime_emitter/make_datum_float64.cxx
            src/c4rt2c/runtime_emitter/make_datum_int64.cxx
            src/c4rt2c/runtime_emitter/make_datum_nil.cxx
            src/c4rt2c/runtime_emitter/make_datum_str.cxx
            src/c4rt2c/runtime_emitter/make_seq_thunk.cxx
            src/c4rt2c/runtime_emitter/make_thunk.cxx
            src/c4rt2c/runtime_emitter/merge_argv.cxx
            src/c4rt2c/runtime_emitter/runtime_emitter.cxx
            src/c4rt2c/runtime_emitter/set_completion_K.cxx
            src/c4rt2c/runtime_emitter/set_completion_self.cxx
            src/c4rt2c/runtime_emitter/set_completion_thunk.cxx
            src/c4rt2c/runtime_emitter/set_datum_argv.cxx
            src/c4rt2c/runtime_emitter/set_datum_argv_sz.cxx
            src/c4rt2c/runtime_emitter/set_datum_type.cxx
            src/c4rt2c/runtime_emitter/set_datum_value.cxx
            src/c4rt2c/runtime_emitter/set_thunk_args.cxx
            src/c4rt2c/runtime_emitter/tail_call.cxx

            include/c4rt2c/seq_node.hxx
            src/c4rt2c/seq_node/seq_leaf/build.cxx
            src/c4rt2c/seq_node/seq_leaf/push.cxx
            src/c4rt2c/seq_node/seq_leaf/seq_leaf.cxx
            src/c4rt2c/seq_node/seq_nil/build.cxx
            src/c4rt2c/seq_node/seq_nil/push.cxx
            src/c4rt2c/seq_node/seq_pair/build.cxx
            src/c4rt2c/seq_node/seq_pair/last.cxx
            src/c4rt2c/seq_node/seq_pair/push.cxx
            src/c4rt2c/seq_node/seq_pair/seq_pair.cxx

            include/c4rt2c/seq_builder.hxx
            src/c4rt2c/seq_builder/seq_builder.cxx
            src/c4rt2c/seq_builder/push.cxx
            src/c4rt2c/seq_builder/build.cxx
            src/c4rt2c/seq_builder/~seq_builder.cxx
            src/c4rt2c/ast2_ir_emitter/define_native_function.cxx
            src/c4rt2c/ast2_ir_emitter/define_bridge_function.cxx
            src/c4rt2c/runtime_emitter/continue_with.cxx
            src/c4rt2c/runtime_emitter/unpack_argv.cxx
            src/c4rt2c/runtime_emitter/make_forces_blob.cxx
            src/c4rt2c/runtime_emitter/force_args.cxx
            src/c4rt2c/runtime_emitter/set_tailkind.cxx
            src/c4rt2c/runtime_emitter/init.cxx
            src/c4rt2c/runtime_emitter/fn_type.cxx

            include/c4rt2c/fn_call_gen.hxx
            src/c4rt2c/fn_call_gen/immediate_fn_call_gen/resolve_symbol_value.cxx
            src/c4rt2c/fn_call_gen/direct_fn_call_gen/build.cxx
            src/c4rt2c/fn_call_gen/direct_fn_call_gen/refer.cxx
            src/c4rt2c/fn_call_gen/immediate_fn_call_gen/finalize.cxx
            src/c4rt2c/fn_call_gen/direct_fn_call_gen/resolve_symbol_value.cxx
            src/c4rt2c/fn_call_gen/tail_fn_call_gen/resolve_symbol_value.cxx
            src/c4rt2c/fn_call_gen/tail_fn_call_gen/finalize.cxx
            src/c4rt2c/seq_node/seq_leaf/build_call.cxx
            src/c4rt2c/seq_node/seq_pair/build_call.cxx
            src/c4rt2c/seq_node/seq_nil/build_call.cxx
            src/c4rt2c/seq_node/seq_tail_leaf/push.cxx
            src/c4rt2c/seq_node/seq_tail_leaf/build.cxx
            src/c4rt2c/seq_node/seq_tail_leaf/build_call.cxx
            src/c4rt2c/runtime_emitter/tail_seq.cxx
            src/c4rt2c/runtime_emitter/make_seq_argv.cxx
            src/c4rt2c/ast2_ir_emitter/bool_attr.hxx
            src/c4rt2c/seq_node/seq_pair/build_immediate.cxx
            src/c4rt2c/seq_node/seq_tail_leaf/build_immediate.cxx
            src/c4rt2c/runtime_emitter/make_seq_ti_argv.cxx
            src/c4rt2c/seq_node/seq_leaf/build_immediate.cxx
            )
target_link_libraries(c4rt2c
                      PUBLIC
                      c4-mio
                      PRIVATE
                      c4
                      c4_llvm
                      libassert::assert)
target_include_directories(c4rt2c
                           PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>)
