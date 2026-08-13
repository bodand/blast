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
#include <thread>

#include <c4rt3/c4rt.h>

#include <c4/array.h>

#include <gc/gc.h>

#include <clang/ASTMatchers/ASTMatchFinder.h>
#include <clang/ASTMatchers/Dynamic/Diagnostics.h>
#include <clang/ASTMatchers/Dynamic/Parser.h>
#include <clang/Frontend/ASTUnit.h>
#include <clang/Tooling/Tooling.h>

#include "../compilation_db.hxx"
#include "../ext-type.hxx"
#include "../handler/diagnostic_handler.hxx"
#include "../handler/handler_base.hxx"

namespace ast = clang::ast_matchers;
namespace dyn = clang::ast_matchers::dynamic;

namespace fs = std::filesystem;

namespace {
	struct draining_work_pool final {
		explicit draining_work_pool(const std::vector<fs::path>& files)
			: _files{files} { }

		std::optional<fs::path>
		get_work() {
			std::scoped_lock lck(_files_mx);
			if (_files.empty()) return std::nullopt;
			const auto ret = _files.back();
			_files.pop_back();
			return ret;
		}

	private:
		std::vector<fs::path> _files;
		std::mutex _files_mx{};
	};

	struct blast_callback final : ast::MatchFinder::MatchCallback {
		explicit
		blast_callback(const c4_array handlers) {
			_handlers.reserve(handlers->len + 1);

			for (size_t i = 0; i < handlers->len; ++i) {
				const bst::handler_base* handler = nullptr;
				c4_datum_get_handler(handlers->data[i], &handler);
				if (handler) _handlers.emplace_back(handler->clone());
			}
			_handlers.emplace_back(new bst::fallback_diagnostic_handler);
		}

		blast_callback(const blast_callback& other) = delete;

		blast_callback(blast_callback&& other) noexcept = delete;

		blast_callback&
		operator=(const blast_callback& other) = delete;

		blast_callback&
		operator=(blast_callback&& other) noexcept = delete;

		void
		reset() {
			_matches.clear();
			_context = nullptr;
		}

		void
		match(clang::ast_matchers::MatchFinder& finder) {
			finder.matchAST(_context->getASTContext());
			run_handlers();
			reset();
		}

		void
		run(const ast::MatchFinder::MatchResult& result) override {
			for (const auto& [id, node] : result.Nodes.getMap()) {
				_matches.emplace_back(id, node);
			}
		}

		void
		run_handlers() {
			for (const auto& [name, node] : _matches) {
				handle(name, node);
			}
		}

		void
		set_ast(std::unique_ptr<clang::ASTUnit> unit) {
			_context = std::move(unit);
		}

		void
		dump_handlers(llvm::raw_ostream& errs) const {
			std::ranges::for_each(_handlers, [&](const auto& handler) {
				handler->dump_diagnostics(errs);
			});
		}

	private:
		void
		handle(const std::string_view name,
		       const clang::DynTypedNode& node) {
			for (const auto& handler : _handlers) {
				if (handler->try_handle(name, _context, node)) break;
			}
		}

		std::vector<std::unique_ptr<bst::handler_base>> _handlers;
		std::unique_ptr<clang::ASTUnit> _context = nullptr;
		std::vector<std::pair<std::string, clang::DynTypedNode>> _matches;
	};

	struct matcher_worker {
		explicit
		matcher_worker(const c4_array handlers,
		               const auto& matcher,
		               bst::compilation_db* db,
		               draining_work_pool* pool)
			: _callback(handlers)
			, _db{db}
			, _pool{pool} {
			if (_finder.addDynamicMatcher(matcher, &_callback)) {
				_running.test_and_set();
			}
		}

		void
		operator()() try {
			if (!_running.test()) return;
			GC_stack_base base;
			GC_get_stack_base(&base);
			GC_register_my_thread(&base);

			// hosted out of loop as to not consistently reallocate 1 sized arrays
			std::vector<std::unique_ptr<clang::ASTUnit>> units;

			for (auto work = _pool->get_work();
			     work;
			     work = _pool->get_work()) {
				const auto file = *work;

				units.clear();

				auto tool = _db->build_tool(file);
				tool.buildASTs(units);

				if (units.empty()
				    || !units.front()
				    || units.front()->getDiagnostics().hasErrorOccurred()) {
					std::cerr << "blast: error: cannot build AST for "
							<< file << std::endl;
					continue;
				}

				auto&& unit = units.front();
				_callback.set_ast(std::move(unit));
				_callback.match(_finder);
			}

			GC_unregister_my_thread();
		}
		catch (...) {
			GC_unregister_my_thread();
		}

		void
		dump() const {
			_callback.dump_handlers(llvm::errs());
		}

	private:
		std::atomic_flag _running = ATOMIC_FLAG_INIT;
		ast::MatchFinder _finder{};
		blast_callback _callback;
		bst::compilation_db* _db;
		draining_work_pool* _pool;
	};
}

c4_let_native(blast_match_ast)(
	const c4_datum ast_db,
	const c4_datum matcher_str,
	const c4_datum handlers_array
) {
	char* matcher;
	size_t matcher_sz;
	c4_datum_coerce_string(matcher_str, &matcher, &matcher_sz);
	llvm::StringRef matcher_code(matcher, matcher_sz);

	bst::compilation_db* db;
	c4_datum_get_db(ast_db, &db);

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

	c4_array handlers;
	c4_datum_get_array(handlers_array, &handlers);

	draining_work_pool pool(db->files());

	std::vector<std::unique_ptr<matcher_worker>> workers;
	workers.reserve(std::thread::hardware_concurrency());
	std::generate_n(std::back_inserter(workers),
	                std::thread::hardware_concurrency(),
	                [&] {
		                return std::make_unique<matcher_worker>(
			                handlers,
			                final,
			                db,
			                &pool);
	                });

	std::vector<std::thread> threads;
	threads.reserve(workers.size());
	std::ranges::transform(
		workers, std::back_inserter(threads),
		[](auto& worker) {
			return std::thread(&matcher_worker::operator(), worker.get());
		});

	std::ranges::for_each(threads, [](auto& thread) { thread.join(); });
	std::ranges::for_each(workers, [](auto& worker) { worker->dump(); });

	c4_datum nil;
	c4_datum_from_nil(&nil);
	return nil;
}
