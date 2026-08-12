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
 * src/blast/src/blast/blast_match_ast --
 *   
 */

#include <iostream>
#include <c4rt3/c4rt.h>

#include <c4/array.h>

#include <clang/ASTMatchers/ASTMatchFinder.h>
#include <clang/ASTMatchers/Dynamic/Diagnostics.h>
#include <clang/ASTMatchers/Dynamic/Parser.h>
#include <clang/Frontend/ASTUnit.h>

#include "../ext-type.hxx"
#include "../handler/diagnostic_handler.hxx"
#include "../handler/handler_base.hxx"

namespace ast = clang::ast_matchers;
namespace dyn = clang::ast_matchers::dynamic;

namespace {
	struct blast_callback final : ast::MatchFinder::MatchCallback {
		explicit
		blast_callback(const c4_datum handlers_array) {
			c4_array handlers;
			c4_datum_get_array(handlers_array, &handlers);
			_handlers.reserve(handlers->len + 1);

			for (size_t i = 0; i < handlers->len; ++i) {
				bst::handler_base* handler = nullptr;
				c4_datum_get_handler(handlers->data[i], &handler);
				if (handler) _handlers.push_back(handler);
			}
			_handlers.push_back(&_fallback_handler);
		}

		void run(const ast::MatchFinder::MatchResult& result) override {
			for (const auto& [id, node] : result.Nodes.getMap()) {
				handle(id, result.Context, node);
			}
		}

	private:
		void
		handle(const std::string_view name,
		       clang::ASTContext* context,
		       const clang::DynTypedNode& node) const {
			for (const auto& handler : _handlers) {
				if (handler->try_handle(name, context, node)) break;
			}
		}

		std::vector<bst::handler_base*> _handlers;
		bst::fallback_diagnostic_handler _fallback_handler;
	};
}

c4_let_native(blast_match_ast)(
	c4_datum ast_datum_arr,
	c4_datum matcher_str,
	c4_datum handlers_array) {
	char* matcher;
	size_t matcher_sz;
	c4_datum_coerce_string(matcher_str, &matcher, &matcher_sz);
	llvm::StringRef matcher_code(matcher, matcher_sz);

	c4_array units_array;
	c4_datum_get_array(ast_datum_arr, &units_array);

	std::vector<clang::ASTUnit*> units;
	units.reserve(units_array->len);

	for (size_t i = 0; i < units_array->len; ++i) {
		clang::ASTUnit** unit_ptr;
		c4_datum_get_ast_unit(units_array->data[i], &unit_ptr);
		units.push_back(*unit_ptr);
	}

	dyn::Diagnostics diags;
	const auto m = dyn::Parser::parseMatcherExpression(matcher_code, &diags);
	if (!m) {
		std::cerr << "blast: fatal: error parsing matcher expression: "
				<< diags.toStringFull() << std::endl;
		c4_datum nil;
		c4_datum_from_nil(&nil);
		return nil;
	}

	const auto bound = m->tryBind("root");
	const auto& final = bound ? *bound : *m;

	blast_callback cb(handlers_array);
	ast::MatchFinder finder;
	if (!finder.addDynamicMatcher(final, &cb)) {
		std::cerr << "blast: fatal: "
				"matcher's top-level kind can't be matched against a TU"
				<< std::endl;
		c4_datum nil;
		c4_datum_from_nil(&nil);
		return nil;
	}

	std::ranges::for_each(units, [&finder](auto* unit) {
		finder.matchAST(unit->getASTContext());
	});

	c4_datum nil;
	c4_datum_from_nil(&nil);
	return nil;
}
