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
 * Originally created: 2025-03-03.
 *
 * src/c4/include/c4/ast2/visitor/typeid --
 *   
 */
#ifndef C4_AST2_VISITOR_TYPEID_HXX
#define C4_AST2_VISITOR_TYPEID_HXX

#include <cstdint>

namespace c4::ast2::visitor_aux {
    struct type_id final {
        template<class T>
        static type_id
        of() {
            return type_id(my_type<T>::create(type_cntr));
        }

        bool operator==(const type_id& other) const noexcept = default;

        bool operator!=(const type_id& other) const noexcept = default;

    private:
        std::uint_fast32_t _value = 0;
        inline static std::uint_fast32_t type_cntr = 1;

        explicit
        type_id(const std::uint_fast32_t value)
            : _value{value} { }

        template<class>
        struct my_type {
            static std::uint_fast32_t create(std::uint_fast32_t& cnt) {
                if (_id == 0) _id = cnt++;
                return _id;
            }
        private:
            inline static std::uint_fast32_t _id = 0;
        };
    };
}

#endif
