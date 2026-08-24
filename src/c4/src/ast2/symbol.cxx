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
 * src/c4/src/ast2/symbol --
 *   
 */

#include <algorithm>
#include <ranges>
#include <string>
#include <string_view>

#include <c4/ast2/ast_context.hxx>
#include <c4/ast2/symbol.hxx>

#include <fmt/base.h>

#include <libassert/assert.hpp>

using namespace std::literals;

c4::ast2::symbol::symbol(const c4::position& position,
                         const std::string_view name,
                         const unsigned arity)
	: source_positioned{position}
	, _name{name}
	, _arity{arity} {
	DEBUG_ASSERT(!_name.empty(), "symbol name must not be empty");
}

namespace {
    // clang-format off
    constexpr auto operator_chars =            "-+*/%&|~!?,.:^@#`<>="sv;
    constexpr auto operator_char_replacement = "mptsPaoTeqcdChAHblgE"sv;
	// clang-format on
	static_assert(operator_chars.size() == operator_char_replacement.size(),
	              "replacement set must equal operator set");

	constexpr bool
	is_operator_symbol(const std::string_view name) {
		const auto idx = name.find_first_of(operator_chars);
		return idx != std::string_view::npos;
	}

	std::string
	mangle_standard_function(const std::string_view name,
	                         const unsigned arity) {
		return fmt::format("s{}N{}{}E",
		                   name.size(),
		                   name,
		                   arity);
	}

	constexpr char
	translate_operator_char(const char c) {
		const auto c_idx = operator_chars.find(c);
		if (c_idx == std::string_view::npos) return c;
		return operator_char_replacement[c_idx];
	}

	std::string
	mangle_operator(const std::string_view name,
	                const unsigned arity) {
		auto normalized = std::string(name);
		std::ranges::transform(normalized, normalized.begin(), translate_operator_char);
		return fmt::format("o{}N{}{}E",
		                   normalized.size(),
		                   normalized,
		                   arity);
	}

	std::string
	mangle_symbol(const std::string_view name,
	              const unsigned arity) {
		if (is_operator_symbol(name)) return mangle_operator(name, arity);
		return mangle_standard_function(name, arity);
	}
}

std::string
c4::ast2::undef_symbol::mangle() const {
	return mangle_symbol(_name, _arity);
}

c4::ast2::symbol
c4::ast2::symbol::with_arity(const unsigned arity) const {
	auto cpy = *this;
	cpy._arity = arity;
	return cpy;
}

std::string
c4::ast2::symbol::mangle() const {
	if (native()) return fmt::format("c4sym_{}", _name);
	return mangle_symbol(_name, _arity);
}

bool
c4::ast2::symbol::captured() const noexcept {
	if (extern_()) return false;
	DEBUG_ASSERT(_references != nullptr,
	             "captured symbol must have a referable as per !extern_()",
	             _name,
	             _arity,
	             mangle());

	return !_references->value_constant();
}

void
c4::ast2::symbol::references(tags::referable* ref) const noexcept {
	if (ref == nullptr) {
		_references = nullptr;
		return;
	}

	DEBUG_ASSERT(ref->name() == _name,
	             "referenced entity must have the same name");
	DEBUG_ASSERT((!_references || _references == ref),
	             "referenced value should not be overwritten");
	_references = ref;
}

std::string
c4::ast2::symbol::pretty() const {
	if (!is_operator_symbol(_name)) return std::format("{}/{}", _name, _arity);
	if (_arity == 1) return std::format("({})/{}", _name, _arity);

	return std::format("({})/{} {} {}",
		_name,
		_arity,
		_op_data->left_associative ? "left" : "right",
		_op_data->precedence);
}

void
c4::ast2::symbol::lift_to_context(ast_context& ctx) {
	const auto name2 = ctx.lift_symbol(_name);
	_name = name2;
}
