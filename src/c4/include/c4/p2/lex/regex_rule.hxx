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
 * src/c4/include/c4/p2/lex/regex --
 *   Wrapper over a regex library to provide regex based matching.
 *   All of these classes are tightly coupled, but this is by design: they
 *   are different classes to encapsulate the different resource management
 *   needs, but cannot work by themselves anyways since the backing
 *   implementation (e.g. PCRE2) objects work this way.
 */
#ifndef C4_REGEX_RULE_HXX
#define C4_REGEX_RULE_HXX

#include <utility>
#include <cstdint>
#include <optional>
#include <span>
#include <string_view>

#include <c4/p2/lex/token_source.hxx>
#include <c4/p2/lex/token_traits.hxx>
#include <c4/p2/lex/tokens.hxx>

namespace c4 {
	struct position;
}

namespace c4::p2 {
	struct regex_context final {
		regex_context();

		regex_context(regex_context& cp) = delete;

		regex_context&
		operator=(regex_context& cp) = delete;

		regex_context(regex_context&& mv) noexcept
			: _impl{std::exchange(mv._impl, nullptr)} { }

		regex_context&
		operator=(regex_context&& mv) noexcept {
			_impl = std::exchange(mv._impl, _impl);
			return *this;
		}

		~regex_context() noexcept;

	private:
		friend struct regex_rule;
		void* _impl;
	};

	struct regex_match_context final {
		explicit
		regex_match_context(std::uint32_t capture_groups);

		regex_match_context(regex_match_context& cp) = delete;

		regex_match_context&
		operator=(regex_match_context& cp) = delete;

		~regex_match_context() noexcept;

	private:
		friend struct regex_rule;
		void* _impl{};
	};

	struct regex_rule final {
		constexpr static std::uint32_t max_capture_groups = 4;

		regex_rule(token_source& token_source,
		           std::string_view regex,
		           regex_context& ctx,
		           std::uint32_t groups_expected);

		regex_rule(regex_rule&& mv) noexcept
			: _token_source{std::exchange(mv._token_source, nullptr)}
			, _ctx{std::exchange(mv._ctx, nullptr)}
			, _impl_handle{std::exchange(mv._impl_handle, nullptr)} { }

		regex_rule&
		operator=(regex_rule&& mv) noexcept {
			_token_source = std::exchange(mv._token_source, _token_source);
			_ctx = std::exchange(mv._ctx, _ctx);
			_impl_handle = std::exchange(mv._impl_handle, _impl_handle);
			return *this;
		}

		~regex_rule() noexcept;

		template<class T>
		std::optional<T>
		match(const char*& data, const char* end, position& pos) const {
			regex_match_context match_context{token_capture_groups<T>::value};
			std::array<std::string_view, token_capture_groups<T>::value> captures;

			return match_into(data, end, pos, match_context, captures)
					.and_then([&, this](position matched_at) -> std::optional<T> {
						if constexpr (token_can_match_newline<T>::value) {
							offset_positions_newline(captures[0], data, end, pos);
						}
						else {
							offset_positions_no_newline(captures[0], data, end, pos);
						}
						// pos now points to next character to be read
						matched_at.row_number_end = pos.row_number;
						matched_at.col_number_end = pos.col_number - 1;

						const auto range_start = matched_at.line.data();
						const auto range_end = pos.line.data() + pos.line.size();
						matched_at.expanded_range = std::string_view{range_start, range_end};

						return std::apply([this]<typename... Args>(Args&&... args) {
							                  return _token_source->build<T>(std::forward<Args>(args)...);
						                  },
						                  to_ctor_args_tuple(std::move(matched_at), captures));
					});
		}

	private:
		static void
		offset_positions_newline(std::string_view match,
		                         const char*& begin,
		                         const char* end,
		                         position& pos);

		static void
		offset_positions_no_newline(std::string_view match,
		                            const char*& begin,
		                            const char* end,
		                            position& pos);

		template<std::size_t N>
		static auto
		to_ctor_args_tuple(const position& pos,
		                   const std::array<std::string_view, N>& matches) {
			const auto matches_tuple = [&matches]<std::size_t... Is>(std::index_sequence<Is...>) {
				return std::make_tuple(matches[Is]...);
			}(std::make_index_sequence < N > { }
			)
			;
			return std::tuple_cat(std::make_tuple(pos), matches_tuple);
		}

		std::optional<position>
		match_into(
			const char*& data,
			const char* end,
			const position& pos,
			const regex_match_context& match_context,
			std::span<std::string_view> matches
		) const;

		token_source* _token_source;
		regex_context* _ctx;
		void* _impl_handle{};
	};
}

#endif
