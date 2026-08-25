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
 * Originally created: 2026-08-25.
 *
 * src/c4/include/c4/p2/archive_parser --
 *   
 */
#ifndef BLAST_ARCHIVE_PARSER_HXX
#define BLAST_ARCHIVE_PARSER_HXX

#include <vector>

#include <c4/p2/a/lexer.hxx>

#include "named_source.hxx"

namespace c4 {
	struct diagnostics_engine;
	struct archive_file;
}

namespace c4::ast2 {
	struct let_expression;
	struct ast_context;
}

namespace c4::p2 {
	struct archive_parser {
		archive_parser(ast2::ast_context& ctx,
		               diagnostics_engine& diag,
		               const archive_file& archive);

		std::vector<ast2::let_expression*>
		parse();

	private:
		std::vector<ast2::let_expression*> _expressions;

		diagnostics_engine& _diag;
		ast2::ast_context& _ctx;

		const archive_file& _archive;

		invalid_file_source _named_src;
		a::lexer _lexer;
		a::tokens::token_type _current;
	};
}

#endif
