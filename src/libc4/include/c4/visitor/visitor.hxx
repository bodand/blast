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
 * src/c4/include/c4/visitor/visitor --
 *   
 */
#ifndef C4_AST2_VISITOR_VISITOR_HXX
#define C4_AST2_VISITOR_VISITOR_HXX

#include <c4/visitor/visitor_base.hxx>

namespace c4::ast2 {
	template<class... Ts>
	struct visitor : typed_visitor_base<Ts>... {
		~visitor() override = default;

	protected:
		template<class T>
		C4_AST_ATTR_NODEBUG bool
		visit_one(const void* untyped, const visitor_aux::type_id tid) {
			if (visitor_aux::type_id::of<T>() != tid) return false;
			static_cast<typed_visitor_base<T>*>(this)->do_visit(*static_cast<const T*>(untyped));
			return true;
		}

		C4_AST_ATTR_NODEBUG void
		visit_impl(const void* raw, const visitor_aux::type_id tid) final {
			(visit_one<Ts>(raw, tid) || ...);
		}
	};
}

#endif
