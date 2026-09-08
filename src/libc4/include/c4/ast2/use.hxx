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
 * Originally created: 2026-09-08.
 *
 * src/libc4/include/c4/ast2/use --
 *   
 */
#ifndef C4_AST2_USE_HXX
#define C4_AST2_USE_HXX

#include <c4/ast2/ast_node.hxx>

#include <c4/tags/attributable.hxx>
#include <c4/tags/source_positioned.hxx>
#include <c4/tags/visitable.hxx>

namespace c4::ast2 {
	struct ast_context;

	struct use : ast_node
	           , tags::visitable
	           , tags::attributable
	           , tags::source_positioned {
		explicit
		use(const struct position& position,
		    bool library,
		    std::string_view name);

		use(const use& other) = delete;

		use(use&& other) noexcept = delete;

		use&
		operator=(const use& other) = delete;

		use&
		operator=(use&& other) noexcept = delete;

		[[nodiscard]] bool
		library() const noexcept { return _library; }

		[[nodiscard]] std::string_view
		name() const noexcept { return _name; }

		void
		lift_to_context(ast_context& ctx);

		[[nodiscard]] static unsigned
		unbound_parameters() noexcept { return 0; }

		[[nodiscard]] std::optional<unsigned>
		invocable_with() const noexcept { return std::nullopt; }

		template<class... Args>
		[[nodiscard]] bool
		constant_evaluated(Args&&...) const noexcept { return true; }

	private:
		bool _library;
		std::string_view _name;
	};
}

#endif
