/* blAST project
 *
 * Copyright (c) 2026 András Bodor <bodand@pm.me>
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
 * Originally created: 2026-07-22.
 *
 * src/c4rt2/include/c4rt2c/runtime_emitter --
 *   
 */
#ifndef BLAST_RUNTIME_EMITTER_HXX
#define BLAST_RUNTIME_EMITTER_HXX

#include <cstddef>
#include <cstdint>

#include <c4rt2c/runtime_fn.hxx>

#include <llvm/IR/IRBuilder.h>

namespace llvm {
	class DIBuilder;
	class FunctionType;
	class LLVMContext;
	class Module;
	class Value;
}

namespace c4rt2c {
	struct runtime_emitter {
		runtime_emitter(llvm::LLVMContext& ctx,
		                llvm::Module& module,
		                llvm::IRBuilder<>& builder);

		[[nodiscard]] llvm::FunctionType*
		function_type() const { return _function_type; }

		[[nodiscard]] llvm::Value*
		allocate(llvm::Value* size) const;

		[[nodiscard]] llvm::Value*
		allocate(std::size_t size) const;

		[[nodiscard]] llvm::Value*
		allocate_array(llvm::Value* count, llvm::Value* size) const;

		[[nodiscard]] llvm::Value*
		allocate_array(std::size_t count, std::size_t size) const;

		void
		complete_thunk(llvm::Value* self, llvm::Value* res) const;

		void
		evaluate(llvm::Value* thunk, llvm::Value* K) const;

		[[nodiscard]] llvm::Value*
		make_datum_str(std::string_view str, const std::optional<std::string_view>& global_name = {}) const;

		[[nodiscard]] llvm::Value*
		make_datum_int64(llvm::Value* val) const;

		[[nodiscard]] llvm::Value*
		make_datum_int64(std::int64_t val) const;

		[[nodiscard]] llvm::Value*
		make_datum_nil() const;

		[[nodiscard]] llvm::Value*
		make_datum_float64(llvm::Value* val) const;

		[[nodiscard]] llvm::Value*
		make_datum_float64(double val) const;

		[[nodiscard]] llvm::Value*
		make_datum_block(llvm::Value* val) const;

		[[nodiscard]] llvm::Value*
		make_thunk(llvm::Value* fnptr) const;

		[[nodiscard]] llvm::Value*
		make_apply_thunk(std::span<llvm::Value*> args) const;

		[[nodiscard]] llvm::Value*
		make_seq_thunk(llvm::Value* thunk_left, llvm::Value* thunk_right) const;

		void
		tail_seq(llvm::Value* left, llvm::Value* right, llvm::Value* K) const;

		[[nodiscard]] llvm::Value*
		make_forces_blob(llvm::Value* argv, llvm::Value* argv_sz, llvm::Function* trampoline) const;

		[[nodiscard]] llvm::Value*
		make_forces_blob(llvm::Value* argv, std::uint32_t argv_sz, llvm::Function* trampoline) const;

		llvm::Value*
		merge_argv(llvm::Value* argv1, llvm::Value* argv1_sz, llvm::Value* argv2) const;

		void
		set_thunk_args(llvm::Value* thunk, llvm::Value* argv, llvm::Value* argv_sz) const;

		void
		set_thunk_args(llvm::Value* thunk, llvm::Value* argv, uint32_t argv_sz) const;

		void
		continue_with(llvm::Value* K, llvm::Value* ret) const;

		void
		force_args(llvm::Value* forces, llvm::Value* K) const;

		llvm::SmallVector<llvm::Value*, 4>
		unpack_argv(llvm::Value* forces, std::uint32_t argv_sz) const;

		void
		tail_call(llvm::Value* callee, llvm::Value* args, llvm::Value* K) const;

		void
		tail_call(llvm::FunctionCallee callee, llvm::Value* args, llvm::Value* K) const;

		void
		set_debug_trace(const bool debug_trace) {
			debug.break_tco = debug_trace;
		}

		void
		init(llvm::DIBuilder* dib, llvm::DICompileUnit* cu);

		llvm::DISubroutineType*
		fn_type(size_t cls_size, size_t argv_sz) const;

	private:
		llvm::LLVMContext& _context;
		llvm::Module& _module;
		llvm::IRBuilder<>& _builder;

		llvm::PointerType* ptr_t;
		llvm::IntegerType* int8_t;
		llvm::IntegerType* int32_t;
		llvm::IntegerType* int64_t;

		llvm::StructType* datum_t;
		llvm::StructType* completion_t;
		llvm::StructType* args_force_t;

		struct debug {
			bool break_tco{};
		} debug{};

		llvm::DIDerivedType* _dbg_ptr_t;
		llvm::DIBasicType* _dbg_int8_t;
		llvm::DIBasicType* _dbg_argv_size_t;
		llvm::DIBasicType* _dbg_int64_t;
		llvm::DIBasicType* _dbg_size_t;
		llvm::DIBasicType* _dbg_uint32_t;
		llvm::DIBasicType* _dbg_uint64_t;
		llvm::DIDerivedType* _dbg_charptr_t;
		llvm::DIBasicType* _dbg_float64_t;
		llvm::DIDerivedType* _dbg_datum_ptr;
		llvm::DIBuilder* _di_builder;

		constexpr static int datum_type_int64 = 0;
		constexpr static int datum_type_thunk = 1;
		constexpr static int datum_type_immediate = 2;
		constexpr static int datum_type_float64 = 3;
		constexpr static int datum_type_string = 4;
		constexpr static int datum_type_block = 5;
		constexpr static int datum_type_nil = 6;

		constexpr static int datum_field_type = 0;
		constexpr static int datum_field_argv_sz = 1;
		constexpr static int datum_field_value = 2;
		constexpr static int datum_field_argv = 3;

		constexpr static int completion_field_self = 0;
		constexpr static int completion_field_payload = 1;
		constexpr static int completion_field_K = 2;

		constexpr static int args_force_field_to_force = 0;
		constexpr static int args_force_field_argv_sz = 1;
		constexpr static int args_force_field_native = 2;
		constexpr static int args_force_field_argv = 3;

		template<class T>
		static T*
		with_name(T* val, std::string_view name) {
			val->setName(name);
			return val;
		}

		llvm::Value*
		get_datum_type(llvm::Value* d) const;

		llvm::Value*
		get_datum_value(llvm::Value* d, llvm::Type* type) const;

		llvm::Value*
		get_datum_argv(llvm::Value* d) const;

		llvm::Value*
		get_datum_argv_sz(llvm::Value* d) const;

		void
		set_datum_type(llvm::Value* d, llvm::Value* type) const;

		void
		set_datum_value(llvm::Value* d, llvm::Value* value) const;

		void
		set_datum_argv(llvm::Value* d, llvm::Value* argv) const;

		void
		set_datum_argv_sz(llvm::Value* d, llvm::Value* argv_sz) const;

		llvm::Value*
		get_completion_self(llvm::Value* c) const;

		llvm::Value*
		get_completion_payload(llvm::Value* c) const;

		llvm::Value*
		get_completion_K(llvm::Value* c) const;

		void
		set_completion_self(llvm::Value* c, llvm::Value* self) const;

		void
		set_completion_payload(llvm::Value* c, llvm::Value* thunk) const;

		void
		set_completion_K(llvm::Value* c, llvm::Value* K) const;

		void
		set_tailkind(llvm::CallInst* call) const;

		[[nodiscard]] llvm::Value*
		make_seq_argv(llvm::Value* left, llvm::Value* right) const;

		/// The universal function type to allow unrestricted
		/// tail-calls. It is void(ptr, ptr), where the first is an array
		/// to pointers as "argv" and the latter is the K continuation.
		llvm::FunctionType* _function_type;

		runtime_fn _rt_allocate_array;     // void* allocate_array(size, size)
		runtime_fn _rt_apply;              // void apply(datum* thunk, fn K)
		runtime_fn _rt_apply2;             // void apply2(completion* self, datum* val)
		runtime_fn _rt_complete_thunk;     // void complete_thunk(datum* thunk, fn K)
		runtime_fn _rt_evaluate;           // void evaluate(datum* thunk, fn K)
		runtime_fn _rt_force_args;         // void force_args(args_force* forces, fn K)
		runtime_fn _rt_force_args2;        // void force_args2(completion* self, datum* val)
		runtime_fn _rt_make_datum_block;   // datum* make_datum_block(fn anon)
		runtime_fn _rt_make_datum_float64; // datum* make_datum_float64(f64 val)
		runtime_fn _rt_make_datum_int64;   // datum* make_datum_int64(i64 val)
		runtime_fn _rt_make_datum_nil;     // datum* make_datum_nil()
		runtime_fn _rt_make_datum_str;     // datum* make_datum_str(char* val)
		runtime_fn _rt_make_thunk;         // datum* make_thunk(fn namedfn)
		runtime_fn _rt_seq;                // datum* seq(datum* thunks, fn K)
		runtime_fn _rt_seq2;               // datum* seq2(completion* self, datum* val)
		runtime_fn _rt_set_thunk_args;     // void set_thunk_args(datum* datum,
		;                                  //                     datum* argv, i32 argv_sz)
		runtime_fn _rt_merge_argv;         // datum* merge_argv(datum* argv1, i32 argv1_sz,
		;                                  //                   datum* argv2, i32 argv2_sz)

		runtime_fn _gc_malloc;
	};
}

#endif
