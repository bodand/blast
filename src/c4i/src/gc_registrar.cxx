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
 * Originally created: 2026-03-03.
 *
 * src/c4i/src/gc_registrar --
 *   
 */

#include <gc_registrar.hxx>

#include <llvm/ADT/BitVector.h>
#include <llvm/ExecutionEngine/Orc/Shared/MemoryFlags.h>

void
c4i::gc_registrar::modifyPassConfig(llvm::orc::MaterializationResponsibility& MR,
                                    llvm::jitlink::LinkGraph& /*G*/,
                                    llvm::jitlink::PassConfiguration& Config) {
    Config.PostAllocationPasses.emplace_back(
        [this, &MR](llvm::jitlink::LinkGraph& G) {
            using namespace llvm; // Get the darned & a few lines below

            for (auto& section : G.sections()) {
                // Ignore non-writable memory
                if ((section.getMemProt() & orc::MemProt::Write) == orc::MemProt::None) continue;

                for (const auto* block : section.blocks()) {
                    const auto block_start = std::bit_cast<void*>(block->getAddress().getValue());
                    const auto block_end = std::bit_cast<void*>(block->getAddress().getValue() + block->getSize());
                    assert(block_start <= block_end);

                    auto error = MR.withResourceKeyDo(
                        [this, block_end, block_start](const orc::ResourceKey K) {
                            put_root(K, block_start, block_end);
                        }
                    );
                    if (!error) std::ignore = error;

                    GC_add_roots(block_start, block_end);
                }
            }
            return Error::success();
        }
    );
}

llvm::Error
c4i::gc_registrar::notifyFailed(llvm::orc::MaterializationResponsibility& MR) {
    return llvm::Error::success();
}

llvm::Error
c4i::gc_registrar::notifyRemovingResources(llvm::orc::JITDylib& JD,
                                           llvm::orc::ResourceKey K) {
    std::scoped_lock<std::mutex> lck(_roots_mx);

    const auto it = _roots.find(K);
    if (it == _roots.end()) return llvm::Error::success();

    for (const auto [start, end] : it->second) {
        GC_remove_roots(start, end);
    }

    _roots.erase(it);

    return llvm::Error::success();
}

void
c4i::gc_registrar::notifyTransferringResources(llvm::orc::JITDylib& JD,
                                               llvm::orc::ResourceKey DstKey,
                                               llvm::orc::ResourceKey SrcKey) {
    std::scoped_lock<std::mutex> lck(_roots_mx);

    const auto it = _roots.find(SrcKey);
    if (it == _roots.end()) return;

    unsafe_put_range(DstKey, it->second.begin(), it->second.end());
}

void
c4i::gc_registrar::unsafe_append_root(const map_type::iterator it, void* begin, void* end) {
    auto& key_associated_roots = it->second;
    key_associated_roots.emplace_back(begin, end);
}

void
c4i::gc_registrar::unsafe_append_range(const map_type::iterator it,
                                       const vector_type::iterator begin,
                                       const vector_type::iterator end) {
    auto& key_associated_roots = it->second;
    key_associated_roots.append(begin, end);
}

c4i::gc_registrar::map_type::iterator
c4i::gc_registrar::unsafe_insert_resource(llvm::orc::ResourceKey K) {
    auto [it, succ] = _roots.try_emplace(K);
    return it;
}

void
c4i::gc_registrar::put_root(const llvm::orc::ResourceKey K, void* begin, void* end) {
    std::scoped_lock<std::mutex> lck(_roots_mx);

    auto it = _roots.find(K);
    if (it == _roots.end()) it = unsafe_insert_resource(K);

    unsafe_append_root(it, begin, end);
}

void
c4i::gc_registrar::unsafe_put_range(const llvm::orc::ResourceKey K,
                                    const vector_type::iterator begin,
                                    const vector_type::iterator end) {
    auto it = _roots.find(K);
    if (it == _roots.end()) it = unsafe_insert_resource(K);

    unsafe_append_range(it, begin, end);
}
