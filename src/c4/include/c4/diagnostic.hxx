/* demo project
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
 * src/c4/include/c4/diagnostic --
 *   Diagnostic object. Can be printed with fmt to produce mostly nice looking
 *   diagnostic messages.
 */
#ifndef C4_DIAGNOSTIC_HXX
#define C4_DIAGNOSTIC_HXX

#include <string_view>
#include <cstdint>
#include <string>
#include <type_traits>
#include <utility>

#include <fmt/base.h>

namespace c4 {
    struct position {
        std::string_view line;
        std::size_t row_number{1};
        std::size_t col_number{1};

        [[nodiscard]] position
        snapshot() const noexcept(std::is_nothrow_copy_constructible_v<position>) {
            return *this;
        }
    };

    struct source_diagnostic {
        enum class diag_type {
            Note,
            Warning,
            Error
        };

        static source_diagnostic
        note(const std::string& diagnostic,
             std::string_view filename,
             const position& position,
             std::size_t highlight_length = 0);

        static source_diagnostic
        note_for_value(const std::string& diagnostic,
                       std::string_view filename,
                       const position& position,
                       std::string_view value);

        static source_diagnostic
        warning(const std::string& diagnostic,
                std::string_view filename,
                const position& position,
                std::size_t highlight_length = 0);

        static source_diagnostic
        warning_for_value(const std::string& diagnostic,
                          std::string_view filename,
                          const position& position,
                          std::string_view value);

        static source_diagnostic
        error(const std::string& diagnostic,
                std::string_view filename,
                const position& position,
                std::size_t highlight_length = 0);

        static source_diagnostic
        error_for_value(const std::string& diagnostic,
                          std::string_view filename,
                          const position& position,
                          std::string_view value);

    private:
        source_diagnostic(const diag_type type,
                          std::string diagnostic,
                          const std::string_view& filename,
                          const position& position,
                          const std::size_t highlight_length = 0)
            : _diagnostic_type{type}
            , _diagnostic{std::move(diagnostic)}
            , _filename{filename}
            , _position{position}
            , _highlight_length{highlight_length} { }

        friend std::string
        format_as(const source_diagnostic& diag);

        diag_type _diagnostic_type;
        std::string _diagnostic;
        std::string_view _filename;
        position _position;
        std::size_t _highlight_length{};
    };

    std::string
    format_as(const source_diagnostic& diag);
}

#endif
