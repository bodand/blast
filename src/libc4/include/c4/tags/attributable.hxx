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
 * src/c4/include/c4/tags/attributable --
 *   
 */
#ifndef C4_AST2_TAGS_ATTRIBUTABLE_HXX
#define C4_AST2_TAGS_ATTRIBUTABLE_HXX

#include <string>
#include <unordered_map>
#include <memory>
#include <expected>
#include <optional>
#include <utility>

#include <c4/visitor/typeid.hxx>

namespace c4::ast2::tags {
	enum class expected_error { NO_VALUE, INVALID_TYPE };

	struct attribute {
		template<class T>
		std::expected<T, expected_error>
		value() {
			return value_untyped(visitor_aux::type_id::of<T>()).and_then([](void* x)
			-> std::expected<T, expected_error> {
					return *static_cast<T*>(x);
				});
		}

		template<class T>
		std::expected<T, expected_error>
		value() const {
			return value_untyped(visitor_aux::type_id::of<T>()).and_then([](const void* x)
			-> std::expected<T, expected_error> {
					return *static_cast<const T*>(x);
				});
		}

		virtual ~attribute() = default;

	protected:
		[[nodiscard]] virtual std::expected<void*, expected_error>
		value_untyped(visitor_aux::type_id tid) = 0;

		[[nodiscard]] virtual std::expected<const void*, expected_error>
		value_untyped(visitor_aux::type_id tid) const = 0;
	};

	template<class T>
	struct typed_attribute : attribute {
		using value_type = T;

	protected:
		explicit
		typed_attribute(const T& value)
			: _value{value} { }

		typed_attribute()
			: _value{} { }

		[[nodiscard]] std::expected<void*, expected_error>
		value_untyped(visitor_aux::type_id tid) final {
			if (tid != visitor_aux::type_id::of<T>()) return std::unexpected{expected_error::INVALID_TYPE};
			if (!_value) return std::unexpected{expected_error::NO_VALUE};
			return &*_value;
		}

		[[nodiscard]] std::expected<const void*, expected_error>
		value_untyped(visitor_aux::type_id tid) const final {
			if (tid != visitor_aux::type_id::of<T>()) return std::unexpected{expected_error::INVALID_TYPE};
			if (!_value) return std::unexpected{expected_error::NO_VALUE};
			return &*_value;
		}

		std::optional<T> _value;
	};

	struct attributable {
		template<class T, class... Args>
			requires std::derived_from<T, attribute>
			         && std::constructible_from<T, Args...>
		std::unique_ptr<attribute>
		emplace_attribute(std::string_view key, Args&&... args) const {
			auto new_attr = std::make_unique<T>(std::forward<Args>(args)...);
			if (const auto it = _attributes.find(key);
				it != _attributes.end())
				return std::unique_ptr<attribute>{std::exchange(it->second, new_attr.release())};
			_attributes.insert(std::make_pair(key, new_attr.release()));
			return {};
		}

		const attribute*
		get_attribute(const std::string_view key) const {
			if (const auto it = _attributes.find(key);
				it != _attributes.end())
				return it->second;
			return nullptr;
		}

		template<class T>
		std::expected<T, expected_error>
		attribute_value(const std::string_view key) const {
			const auto attr = get_attribute(key);
			if (!attr) return std::unexpected{expected_error::NO_VALUE};
			return attr->value<T>();
		}

		const attribute*
		operator[](const std::string_view key) const { return get_attribute(key); }

		attributable() = default;

		attributable(const attributable&) = default;

		attributable&
		operator=(const attributable&) = delete;

		attributable(attributable&&) noexcept = default;

		attributable&
		operator=(attributable&&) noexcept = delete;

		virtual ~attributable();

	private:
		struct string_hash {
			using is_transparent = void;

			[[nodiscard]] size_t
			operator()(const char* txt) const {
				return std::hash<std::string_view>{}(txt);
			}

			[[nodiscard]] size_t
			operator()(const std::string_view txt) const {
				return std::hash<std::string_view>{}(txt);
			}

			[[nodiscard]] size_t
			operator()(const std::string& txt) const {
				return std::hash<std::string>{}(txt);
			}
		};

		mutable std::unordered_map<std::string,
		                           attribute*,
		                           string_hash,
		                           std::equal_to<>> _attributes;
	};
}

#endif
