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
 * Originally created: 2026-08-24.
 *
 * src/c4/src/p2/included_parser/parse_use_expression --
 *   
 */

#include <iostream>
#include <filesystem>

#include <c4/p2/included_parser.hxx>
#include <c4/p2/source_resolver.hxx>

#include "../parser_utils.hxx"

void
c4::p2::included_parser::
parse_use_expression() {
	// Always note the absolute hack that is the use "token".
	// Because otherwise weird parsing rules would break the "" and <> syntaxes
	// the entire darn line is lexed as a single token. Magnificent.
	const auto use = expect_token<tokens::use>();
	if (!use) report_failure(_diag, use);
	next_relevant();

	if (!use->library() && use->is_public()) {
		_diag.error(use->token_position(), "+public use of non-library files is not allowed")
		     .note("parsing as if it were ~internal");
	}

	// always check if file exists to locate errors even if private import
	const auto path = _resolver.resolve(_source,
	                                    use->token_position(),
	                                    use->library(),
	                                    use->path());
	if (!path) return;
	if (use->is_private()) return;

	const auto transitive_src = _resolver.open(*path);
	included_parser nested(_context, _diag, _resolver, transitive_src);
	nested.parse_global_let();

	auto included_exprs = nested.forfeit_expressions();
	std::ranges::for_each(included_exprs, [&](auto& expr) {
		expr->lift_to_context(_context);
	});
	_expressions.reserve(_expressions.size() + included_exprs.size());
	std::ranges::copy(included_exprs, std::back_inserter(_expressions));
}
