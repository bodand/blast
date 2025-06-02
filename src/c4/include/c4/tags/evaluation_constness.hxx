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
 * src/c4/include/c4/tags/evaluation_constness --
 *   Whether a given AST node is a constant value and does not require dynamic
 *   code do generate/use.
 *   This is true for small literals (i32, floats, sso-strings).
 **/
#ifndef C4_AST2_EVALUATION_CONSTNESS_HXX
#define C4_AST2_EVALUATION_CONSTNESS_HXX

namespace c4::ast2::tags {
    struct evaluation_constness {
        [[nodiscard]] bool
        const_evaluable(this auto&& self) noexcept {
            return self.is_constant_evaluable();
        }
    };

    struct dynamic_node : evaluation_constness {
        [[nodiscard]] static consteval bool
        is_constant_evaluable() noexcept { return false; }
    };

    struct constant_node : evaluation_constness {
        [[nodiscard]] static consteval bool
        is_constant_evaluable() noexcept { return true; }

        [[nodiscard]] unsigned
        unbound_parameters() const noexcept {
            // constant nodes do not depend on anything either marked (parameter)
            // or unmarked (closure context), so this is guaranteed 0
            return 0;
        }
    };
}

#endif
