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

#ifndef __clang__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wchanges-meaning
#endif

#include <string_view>
#include <cstdint>
#include <string>
#include <type_traits>
#include <tuple>
#include <utility>
#include <vector>

#include <c4/fmt.hxx>

namespace c4::p2 {
    struct token_source;
}

namespace c4 {
    struct source_diagnostic;
}

template<>
struct fmt::formatter<c4::source_diagnostic> {
    constexpr auto
    parse(format_parse_context& ctx) { return std::ranges::find(ctx, '}'); }

    format_context::iterator
    format(const c4::source_diagnostic& diag, fmt::format_context& ctx) const;
};

namespace c4 {
    struct position {
        static position
        pseudo_position();

        explicit
        position(p2::token_source* source);

        position(const position& other) = default;

        position(position&& other) noexcept = default;

        position&
        operator=(const position& other) = default;

        position&
        operator=(position&& other) noexcept = default;

        std::string_view line;
        std::size_t row_number{1};
        std::size_t col_number{1};
        std::size_t row_number_end{1};
        std::size_t col_number_end{1};
        std::string_view expanded_range{};

        [[nodiscard]] position
        snapshot() const noexcept(std::is_nothrow_copy_constructible_v<position>) {
            return *this;
        }

        [[nodiscard]] std::string_view
        filename() const;

        [[nodiscard]] p2::token_source*
        source() const { return _source; }

        [[nodiscard]] std::string_view
        range() const;

        [[nodiscard]] bool
        is_single_line() const noexcept { return row_number == row_number_end; }

        std::string&
        attach(const std::string_view sv) {
            _attached = std::string{sv};
            return _attached;
        }

    private:
        position(const std::string_view& line_,
                 const std::size_t row_number_,
                 const std::size_t col_number_,
                 const std::size_t row_number_end_,
                 const std::size_t col_number_end_,
                 const std::string_view& expanded_range_,
                 p2::token_source* const source)
            : line{line_}
            , row_number{row_number_}
            , col_number{col_number_}
            , row_number_end{row_number_end_}
            , col_number_end{col_number_end_}
            , expanded_range{expanded_range_}
            , _source{source} { }

        std::string _attached{};
        p2::token_source* _source{};
    };

    struct source_diagnostic {
        enum class diag_type {
            Note,
            Warning,
            Error,
            Suggestion
        };

        template<class... Args>
        [[nodiscard,deprecated("use diagnostics engine")]] static source_diagnostic
        error(const position& position, fmt::format_string<Args...> diagnostic, Args&&... args) {
            return {
                diag_type::Error,
                fmt::format(diagnostic, std::forward<Args>(args)...),
                position
            };
        }

        [[nodiscard]] diag_type
        diagnostic_type() const { return _diagnostic_type; }

        [[nodiscard]] const std::string&
        diagnostic() const { return _diagnostic; }

        [[nodiscard]] std::string_view
        filename() const { return _filename; }

        [[nodiscard]] const position&
        position() const { return _position; }

        source_diagnostic(const diag_type type,
                          std::string diagnostic,
                          const struct position& position)
            : _diagnostic_type{type}
            , _diagnostic{std::move(diagnostic)}
            , _filename{position.filename()}
            , _position{position} { }

        source_diagnostic(const diag_type type,
                          std::string diagnostic,
                          struct position&& position)
            : _diagnostic_type{type}
            , _diagnostic{std::move(diagnostic)}
            , _filename{position.filename()}
            , _position{position} { }

    private:
        diag_type _diagnostic_type;
        std::string _diagnostic;
        std::string_view _filename;
        struct position _position;
    };

    struct diagnostics_bundle {
        ~diagnostics_bundle() noexcept;

        diagnostics_bundle&&
        when(const bool cond) && {
            _skip_next = !cond;
            return std::move(*this);
        }

        template<class P, class... Args>
        diagnostics_bundle&&
        note(P&& position, fmt::format_string<Args...> diagnostic, Args&&... args) &&
            requires(std::same_as<std::remove_cvref_t<P>, struct position>) {
            return std::move(*this).emplace_diagnostic(source_diagnostic::diag_type::Note,
                                                       std::forward<P>(position),
                                                       diagnostic,
                                                       fmt::make_format_args(args...));
        }

        template<class... Args>
        diagnostics_bundle&&
        note(fmt::format_string<Args...> diagnostic, Args&&... args) && {
            return std::move(*this).emplace_diagnostic(source_diagnostic::diag_type::Note,
                                                       _head.position(),
                                                       diagnostic,
                                                       fmt::make_format_args(args...));
        }

        template<class P, class... Args>
        diagnostics_bundle&&
        suggest(P&& position, fmt::format_string<Args...> diagnostic, Args&&... args) &&
            requires(std::same_as<std::remove_cvref_t<P>, struct position>) {
            return std::move(*this).emplace_diagnostic(source_diagnostic::diag_type::Suggestion,
                                                       std::forward<P>(position),
                                                       diagnostic,
                                                       fmt::make_format_args(args...));
        }

        template<class... Args>
        diagnostics_bundle&&
        suggest(fmt::format_string<Args...> diagnostic, Args&&... args) && {
            return std::move(*this).emplace_diagnostic(source_diagnostic::diag_type::Suggestion,
                                                       _head.position(),
                                                       diagnostic,
                                                       fmt::make_format_args(args...));
        }

    private:
        friend struct diagnostics_engine;

        explicit
        diagnostics_bundle(const struct diagnostics_engine& engine,
                           source_diagnostic&& head)
            : _engine(engine)
            , _head{std::move(head)} { }

        diagnostics_bundle&&
        emplace_diagnostic(source_diagnostic::diag_type type,
                           const position& position,
                           fmt::string_view diagnostic,
                           fmt::format_args args) &&;

        diagnostics_bundle&&
        emplace_diagnostic(source_diagnostic::diag_type type,
                           position&& position,
                           fmt::string_view diagnostic,
                           fmt::format_args args) &&;

        const diagnostics_engine& _engine;
        source_diagnostic _head;
        std::vector<source_diagnostic> _tail{};
        bool _skip_next{false};
    };

    struct diagnostics_engine {
        explicit
        diagnostics_engine(std::FILE* const output = stderr,
                           const bool color = true) noexcept
            : _output{output}
            , _color{color} {
            std::ignore = _color; // todo use color
        }

        ~diagnostics_engine() noexcept {
            if (_output == stderr || _output == stdout) return;
            std::fclose(_output);
        }

        [[nodiscard]] bool
        errored() const noexcept { return _errored; }

        template<class... Args>
        decltype(auto)
        warning(const position& position, fmt::format_string<Args...> diagnostic, Args&&... args) {
            return diagnostics_bundle(*this, source_diagnostic{
                                          source_diagnostic::diag_type::Warning,
                                          fmt::format(diagnostic, std::forward<Args>(args)...),
                                          position
                                      });
        }

        template<class... Args>
        decltype(auto)
        error(const position& position, fmt::format_string<Args...> diagnostic, Args&&... args) {
            _errored = true;
            return diagnostics_bundle(*this, source_diagnostic{
                                          source_diagnostic::diag_type::Error,
                                          fmt::format(diagnostic, std::forward<Args>(args)...),
                                          position
                                      });
        }

        void
        emit(const diagnostics_bundle& bundle) const;

    private:
        bool _errored{};
        std::FILE* _output;
        bool _color;
    };
}

#ifndef __clang__
#pragma GCC diagnostic pop
#endif

#endif
