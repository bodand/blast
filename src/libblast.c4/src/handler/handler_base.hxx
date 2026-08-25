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
 * Originally created: 2026-08-11.
 *
 * src/blast/src/handler/handler_base --
 *   
 */
#ifndef BLAST_HANDLER_BASE_HXX
#define BLAST_HANDLER_BASE_HXX

#include <clang/ASTMatchers/ASTMatchFinder.h>
#include <clang/ASTMatchers/Dynamic/Diagnostics.h>
#include <clang/ASTMatchers/Dynamic/Parser.h>
#include <clang/Frontend/ASTUnit.h>

#include "../ext-type.hxx"

namespace bst {
	struct handler_base {
		explicit
		handler_base(const std::string_view name)
			: _name{name} { }

		bool
		try_handle(const std::string_view node_name,
		           std::unique_ptr<clang::ASTUnit>& context,
		           const clang::DynTypedNode& node) {
			if (!should_handle(node_name)) return false;
			return try_handle(context, node);
		}

		virtual ~handler_base() = default;

		virtual void
		dump_diagnostics(llvm::raw_ostream& out) const = 0;

		virtual std::unique_ptr<handler_base>
		clone() const = 0;

		[[nodiscard]] std::string_view
		name() const { return _name; }

	protected:
		virtual bool
		should_handle(const std::string_view name) {
			return _name == name;
		}

		virtual bool
		try_handle(std::unique_ptr<clang::ASTUnit>& context,
		           const clang::DynTypedNode& node) {
			return false;
		}

	private:
		std::string _name;
	};
}

#endif
