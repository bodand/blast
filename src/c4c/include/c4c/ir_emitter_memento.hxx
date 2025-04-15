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
 * Originally created: 2025-04-04.
 *
 * src/c4c/include/c4c/ir_emitter_memento --
 *   
 */
#ifndef C4C_IR_EMITTER_MEMENTO_HXX
#define C4C_IR_EMITTER_MEMENTO_HXX

#include <vector>
#include <memory>

#include <llvm/IR/IRBuilder.h>

namespace c4c {
    struct ir_emitter;

    struct ir_emitter_memento {
        void
        restore(ir_emitter& emitter) const;

    private:
        friend struct ir_emitter;

        ir_emitter_memento(llvm::Function* active_function,
                           const bool function_is_closure,
                           std::vector<llvm::Value*>& need_cleanup,
                           const llvm::IRBuilderBase::InsertPoint& insert_point)
            : _active_function{active_function}
            , _function_is_closure{function_is_closure}
            , _need_cleanup{std::exchange(need_cleanup, {})}
            , _insert_point{insert_point} { }

        llvm::Function* _active_function{};
        bool _function_is_closure{false};
        std::vector<llvm::Value*> _need_cleanup{};
        llvm::IRBuilderBase::InsertPoint _insert_point{};
    };

    struct scoped_memento {
        scoped_memento(std::unique_ptr<ir_emitter_memento>&& memento,
                       ir_emitter& emitter);

        ~scoped_memento() noexcept;
    private:
        std::unique_ptr<ir_emitter_memento> _memento;
        ir_emitter& _emitter;
    };
}

#endif
