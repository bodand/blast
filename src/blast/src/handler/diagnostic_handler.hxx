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
 * src/blast/src/handler/diagnostic_handler --
 *   
 */
#ifndef BLAST_DIAGNOSTIC_HANDLER_HXX
#define BLAST_DIAGNOSTIC_HANDLER_HXX


#include "handler_base.hxx"

#include <clang/Basic/Diagnostic.h>
#include <clang/Frontend/CompilerInstance.h>
#include <clang/Frontend/TextDiagnosticPrinter.h>

#include <llvm/Support/Process.h>
#include <llvm/Support/raw_ostream.h>

namespace bst {
	struct diagnostic_handler : handler_base {
		explicit
		diagnostic_handler(const std::string_view handler_for,
		                   const std::string_view message,
		                   const unsigned id = UINT_MAX)
			: handler_base{handler_for}
			, _message{message}
			, _id{id} {
			const auto opts = llvm::makeIntrusiveRefCnt<clang::DiagnosticOptions>();
			opts->ShowColors = llvm::sys::Process::StandardErrHasColors();

			// printer ownership yoinked by engine
			_printer = new clang::TextDiagnosticPrinter(llvm::errs(), opts.get());
			_engine = clang::CompilerInstance::createDiagnostics(
				opts.get(), _printer);
		}

	protected:
		bool
		try_handle(clang::ASTUnit* unit,
		           const clang::DynTypedNode& node) override {
			_engine->setSourceManager(&unit->getSourceManager());
			_printer->BeginSourceFile(unit->getLangOpts(), nullptr);

			const auto range = node.getSourceRange();
			const auto loc = range.getBegin();
			_engine->Report(loc, id())
					<< _message
					<< clang::CharSourceRange::getTokenRange(range);

			_printer->EndSourceFile();

			return true;
		}

		clang::DiagnosticsEngine&
		engine() const { return *_engine; }

		void
		id(const unsigned id) { _id = id; }

		unsigned
		id() {
			if (_id != UINT_MAX) return _id;
			return _id = _engine->getCustomDiagID(
				       clang::DiagnosticsEngine::Warning,
				       "%0");
		}

		std::string _message;

	private:
		unsigned _id;
		clang::DiagnosticConsumer* _printer = nullptr;
		llvm::IntrusiveRefCntPtr<clang::DiagnosticsEngine> _engine = nullptr;
	};

	struct fallback_diagnostic_handler final : diagnostic_handler {
		explicit
		fallback_diagnostic_handler()
			: diagnostic_handler{"", ""} {
			id(engine().getCustomDiagID(
				clang::DiagnosticsEngine::Warning,
				"unhandled node-binding: %0"));
		}

	protected:
		bool
		should_handle(const std::string_view name) override {
			_message = name;
			return true;
		}
	};
}

#endif
