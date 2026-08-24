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
 * Originally created: 2026-08-23.
 *
 * src/c4/include/c4/p2/parser --
 *   
 */
#ifndef BLAST_PARSER_AUX_HXX
#define BLAST_PARSER_AUX_HXX

#include <expected>
#include <stdexcept>
#include <string_view>
#include <utility>
#include <variant>

#include <c4/diagnostic.hxx>

namespace c4::p2 {
	struct bad_token_error final : std::runtime_error {
		bad_token_error()
			: std::runtime_error("parser failure: bad token") { }
	};

	namespace aux {
		struct mismatched_token_error final {
			std::string_view name;
			std::string_view expected;
			position position;
		};

		struct eof_error final {
			constexpr static std::string_view name = "end of file";
			std::string_view expected;
			position position;
		};

		template<class>
		struct type_val_t { };

		template<class T>
		constexpr static auto type_val = type_val_t<T>{};

		struct token_error final {
			using value_type = std::variant<mismatched_token_error, eof_error>;

			template<class T, class... Args>
			explicit(false)
			token_error(type_val_t<T>, Args&&... args)
				: _value{std::in_place_type<T>, std::forward<Args>(args)...} { }

			explicit
			token_error(value_type value)
				: _value{std::move(value)} { }

			[[nodiscard]] std::string_view
			name() const {
				return std::visit([](const auto& tok) { return tok.name; }, _value);
			}

			[[nodiscard]] std::string_view
			expected_token_name() const {
				return std::visit([](const auto& tok) { return tok.expected; }, _value);
			}

			[[nodiscard]] position
			position() const {
				return std::visit([](const auto& tok) { return tok.position; }, _value);
			}

		private:
			value_type _value;
		};

		template<class T>
		struct token_selector {
			std::expected<T, token_error>
			operator()(const T& tok) const { return tok; }

			template<class Found>
			std::expected<T, token_error>
			operator()(const Found& tok) const {
				return std::unexpected<token_error>(
					std::in_place,
					type_val<mismatched_token_error>,
					Found::token_name,
					T::token_name,
					tok.token_position());
			}
		};
	} //
}

#endif
