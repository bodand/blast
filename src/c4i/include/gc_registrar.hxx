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
 * src/c4i/include/gc_registrar --
 *   
 */
#ifndef BLAST_GC_REGISTRAR_HXX
#define BLAST_GC_REGISTRAR_HXX

#include <gc/gc.h>

#include <llvm/ExecutionEngine/JITLink/JITLink.h>
#include <llvm/ExecutionEngine/Orc/ObjectLinkingLayer.h>

namespace c4i {
struct gc_registrar : llvm::orc::ObjectLinkingLayer::Plugin {
    void
    modifyPassConfig(llvm::orc::MaterializationResponsibility& MR,
                     llvm::jitlink::LinkGraph& G,
                     llvm::jitlink::PassConfiguration& Config) override;

    llvm::Error
    notifyFailed(llvm::orc::MaterializationResponsibility& MR) override;

    llvm::Error
    notifyRemovingResources(llvm::orc::JITDylib& JD,
                            llvm::orc::ResourceKey K) override;

    void
    notifyTransferringResources(llvm::orc::JITDylib& JD,
                                llvm::orc::ResourceKey DstKey,
                                llvm::orc::ResourceKey SrcKey) override;

private:
    struct pointer_range {
        void* begin;
        void* end;
    };

    using vector_type = llvm::SmallVector<pointer_range, 4>;
    using map_type = llvm::DenseMap<llvm::orc::ResourceKey, vector_type>;

    static void
    unsafe_append_root(map_type::iterator it, void* begin, void* end);

    static void
    unsafe_append_range(map_type::iterator it,
                        vector_type::iterator begin,
                        vector_type::iterator end);

    map_type::iterator
    unsafe_insert_resource(llvm::orc::ResourceKey K);

    void
    put_root(llvm::orc::ResourceKey K, void* begin, void* end);

    void
    unsafe_put_range(llvm::orc::ResourceKey K, vector_type::iterator begin, vector_type::iterator end);

    map_type _roots;
    std::mutex _roots_mx;
};
}

#endif
