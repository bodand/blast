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
 * Originally created: 2026-08-22.
 *
 * src/c4ar/src/c4ar --
 *   
 */

#include <filesystem>
#include <iomanip>
#include <iostream>
#include <mutex>
#include <thread>

#include <sgetopt/opts.hxx>
#include <sgetopt/sgetopt.h>

#include <llvm/MC/TargetRegistry.h>
#include <llvm/ObjCopy/ConfigManager.h>
#include <llvm/ObjCopy/ObjCopy.h>
#include <llvm/Object/ArchiveWriter.h>
#include <llvm/Object/ObjectFile.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/TargetParser/Host.h>

#include <libassert/assert.hpp>

namespace fs = std::filesystem;

namespace {
	#define argdesc(flag, arg, ...) "   " #flag "  "  << std::setw(w) << #arg << "   " #__VA_ARGS__ "\n"

	[[noreturn]] void
	usage(const char* argv0) {
		constexpr int w = 7;
		std::cerr << "usage: " << argv0 << " [-dEghIOoT] <object...>\n"
				<< "\n"
				<< "options:\n"
				<< std::left // don't need to restore, we are exiting imminently
				<< argdesc(-h, , Print this help and exit 100.)
				<< argdesc(-j, dir, Add dir for finding C4 library archives.)
				<< argdesc(-T, triplet, Set target triplet to trp. Same format as LLVM.);
		exit(100);
	}

	struct archive_member {
		llvm::SmallVector<char, 4096> raw_buffer{};
		std::string filename{};
		llvm::NewArchiveMember member{};
	};

	struct worker_payload {
		const fs::path& src_obj;
		archive_member& members;
		llvm::SmallVector<char, 4096>& symbol_buffer;
	};

	struct draining_work_pool final {
		explicit
		draining_work_pool(const std::vector<fs::path>* files,
		                   std::vector<archive_member>* object_buffers,
		                   std::vector<llvm::SmallVector<char, 4096>>* symbol_buffers)
			: _files{files}
			, _object_buffers{object_buffers}
			, _symbol_buffers{symbol_buffers} {
			ASSERT(files->size() == object_buffers->size(), "sanity");
			ASSERT(files->size() == symbol_buffers->size(), "sanity");
		}

		std::optional<worker_payload>
		get_work() {
			const size_t next = _files_count.fetch_add(1, std::memory_order::relaxed);
			if (next >= _files->size()) return std::nullopt;

			return worker_payload{
				.src_obj = (*_files)[next],
				.members = (*_object_buffers)[next],
				.symbol_buffer = (*_symbol_buffers)[next],
			};
		}

	private:
		std::atomic<std::size_t> _files_count{0};
		const std::vector<fs::path>* _files;
		std::vector<archive_member>* _object_buffers;
		std::vector<llvm::SmallVector<char, 4096>>* _symbol_buffers;
	};

	struct object_worker {
		explicit
		object_worker(llvm::objcopy::ConfigManager& config)
			: _config{&config} { }

		void
		run(draining_work_pool* pool) const noexcept try {
			while (const auto payload = pool->get_work()) {
				perform(*payload);
			}
		}
		catch (std::exception& e) {
			std::cerr << argv0
					<< ": fatal: object worker failed handling object input: "
					<< e.what() << "\n";
		}
		catch (...) {
			std::cerr << argv0
					<< ": fatal: object worker failed for... unknown reasons? wtf did you do? throw an int?\n";
		}

		explicit operator bool() const noexcept { return !_failed; }

	private:
		void
		perform(const worker_payload& payload) const {
			auto& [src_obj, object_buffer, symbol_buffer] = payload;

			const auto err = llvm::MemoryBuffer::getFile(src_obj.string());
			if (!err) {
				std::cerr << "fatal: cannot open input file " << src_obj.string()
						<< ": " << err.getError().message() <<
						"\n";
				return;
			}
			auto& membuf = err.get();

			auto objerr = llvm::object::ObjectFile::createObjectFile(*membuf);
			if (!objerr) {
				std::cerr << "fatal: invalid data in input file " << src_obj.string()
						<< ": " << toString(objerr.takeError()) <<
						"\n";
				return;
			}
			const auto& obj = objerr.get();

			llvm::raw_svector_ostream os(object_buffer.raw_buffer);
			if (auto writeerr = llvm::objcopy::executeObjcopyOnBinary(*_config, *obj, os)) {
				std::cerr << "fatal: failed to strip  " << src_obj.string()
						<< ": " << toString(std::move(writeerr)) <<
						"\n";
				return;
			}

			/* buf_str is local because it only bridges across types, never
			 * holds any actual resources. The filename however is the actual
			 * data that must not change after member.MemberName points to it,
			 * unless that is also changed accordingly. Do not modify the string
			 * it holds is the general concept.
			 */
			const llvm::StringRef buf_str(object_buffer.raw_buffer.data(),
			                              object_buffer.raw_buffer.size());
			object_buffer.member.Buf = llvm::MemoryBuffer::getMemBuffer(buf_str);
			object_buffer.filename = src_obj.filename().string();
			object_buffer.member.MemberName = object_buffer.filename;

			llvm::SmallVector<llvm::object::SectionRef, 1> sections;
			std::copy_if(
				obj->sections().begin(),
				obj->sections().end(),
				std::back_inserter(sections),
				[&](const llvm::object::SectionRef& sec) {
					auto nameerr = sec.getName();
					if (!nameerr) {
						std::cerr << "warning: corrupted section in " << src_obj.string()
								<< ": " << toString(nameerr.takeError())
								<< ": ignoring it\n";
						return false;
					}
					const auto name = nameerr.get();
					return name == ".c4xport";
				});

			llvm::SmallVector<std::optional<llvm::StringRef>, 1> symbols;
			symbols.reserve(sections.size());
			std::ranges::transform(
				sections, std::back_inserter(symbols),
				[&](const llvm::object::SectionRef& sec) -> std::optional<llvm::StringRef> {
					auto dataerr = sec.getContents();
					if (dataerr) return dataerr.get();

					std::cerr << "error: corrupted section data in " << src_obj.string()
							<< ": " << toString(dataerr.takeError())
							<< ": ignoring it\n";
					return {};
				});

			symbols.erase(
				std::ranges::remove_if(symbols, [](const auto& sym) { return !sym; }).begin(),
				symbols.end());

			if (symbols.size() != sections.size()) {
				_failed = true;
				return; // previously errored
			}

			std::ranges::for_each(symbols, [&symbol_buffer](const auto& sym) {
				ASSERT(sym, "previously filtered");
				std::ranges::copy(*sym, std::back_inserter(symbol_buffer));
			});
		}

		llvm::objcopy::ConfigManager* _config;
		mutable bool _failed = false;
	};
}

int
main(int argc, const char* const* argv) {
	argv0 = argv[0];

	unsigned thread_count = std::thread::hardware_concurrency();
	fs::path out_path = "lib.c4a";
	std::string target_arch;

	subgetopt opts = SUBGETOPT_ZERO;
	opts.prog = argv0;
	for (int opt;
	     (opt = subgetopt_r(argc, argv, "hj:o:T:", &opts)) != -1;) {
		switch (static_cast<char>(opt)) {
		case 'j':
			die_or_parse("-j", opts.arg, thread_count);
			break;
		case 'o':
			out_path = opts.arg;
			break;
		case 'T':
			target_arch = opts.arg;
			break;

		default:
		case 'h':
			usage(argv0);
		}
	}

	argc -= opts.ind;
	if (argc < 1) usage(argv0);
	argv += opts.ind;

	// cap worker threads to input files as to not spawn them just to immediately
	// end them and waste time
	thread_count = std::min(thread_count, static_cast<unsigned>(argc));

	llvm::InitializeAllTargetInfos();
	llvm::InitializeAllTargets();
	llvm::InitializeAllTargetMCs();

	const auto target_triple = target_arch.empty()
		                           ? llvm::sys::getDefaultTargetTriple()
		                           : target_arch;
	const auto llvm_triple = llvm::Triple(target_triple);

	std::string target_error;
	const auto target = llvm::TargetRegistry::lookupTarget(llvm_triple, target_error);
	if (!target) {
		die(2, "{}: fatal: cannot find target triplet {}: {}", target_triple, target_error);
	}

	llvm::objcopy::ConfigManager config;
	auto err = config.Common.ToRemove.addMatcher(
		llvm::objcopy::NameOrPattern::create(".c4xport", llvm::objcopy::MatchStyle::Literal, [](llvm::Error e) {
			return e;
		}));
	if (err) die(2, "{}: fatal: cannot create matcher for .c4xport");

	llvm::SmallVector<object_worker, 16> workers;
	workers.reserve(thread_count);
	std::ranges::generate_n(std::back_inserter(workers), thread_count, [&] {
		return object_worker{config};
	});

	std::vector<fs::path> files;
	files.reserve(argc);
	std::transform(argv, argv + argc, std::back_inserter(files), [](const char* arg) { return arg; });

	std::vector<archive_member> member_buffers;
	member_buffers.resize(files.size());

	std::vector<llvm::SmallVector<char, 4096>> symbol_buffers;
	symbol_buffers.resize(files.size());

	draining_work_pool pool{&files, &member_buffers, &symbol_buffers};

	llvm::SmallVector<std::thread, 16> threads;
	threads.reserve(thread_count);
	std::ranges::transform(workers, std::back_inserter(threads), [&](auto& worker) {
		return std::thread{&object_worker::run, &worker, &pool};
	});

	std::ranges::for_each(threads, std::mem_fn(&std::thread::join));

	std::vector<llvm::NewArchiveMember> members;
	members.reserve(member_buffers.size());
	std::ranges::transform(member_buffers, std::back_inserter(members),
	                       [](auto& member) { return std::move(member.member); });

	auto arerr = llvm::writeArchive(
		out_path.string(),
		members,
		llvm::SymtabWritingMode::NormalSymtab,
		llvm::object::Archive::getDefaultKindForTriple(llvm_triple),
		true,
		false
	);
	if (arerr) {
		std::cerr << "fatal: failed to write archive " << out_path.string() << ": "
				<< toString(std::move(arerr)) << "\n";
		return 111;
	}
}
