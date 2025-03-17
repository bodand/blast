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
 * src/c4/src/value --
 *   
 */

#include <charconv>
#include <format>
#include <sstream>

#include <c4/evaluation_stack.hxx>
#include <c4/value.hxx>
#include <c4/ast.hxx>
#include <c4/block.hxx>

#include <c4/ast/expression.hxx>
#include <c4/ast/block_expression.hxx>

namespace {
    struct evaluator_visitor final {
        const std::shared_ptr<c4::evaluation_stack>& stk;
        std::span<const c4::ast::expression*> args;

        template<class Deleter>
        c4::value
        operator()(const std::unique_ptr<c4::block, Deleter>& op) const {
            return op->evaluate(stk, args);
        }

        c4::value
        operator()(auto&& primitive) const {
            // TODO assert args empty
            return std::forward<decltype(primitive)>(primitive);
        }
    };

    struct value_printer final {
        std::ostream& os;

        std::ostream&
        operator()(const std::int64_t d) const { return os << d; }

        std::ostream&
        operator()(const double d) const { return os << d; }

        std::ostream&
        operator()(const std::string& s) const { return os << s; }

        std::ostream&
        operator()(const c4::symbol& s) const { return os << s.name << "/" << s.arity; }

        std::ostream&
        operator()(const std::unique_ptr<c4::block, c4::block_deleter>& b) const {
            b->print(os);
            return os;
        }
    };
}

void
c4::block_deleter::operator()(block* b) const {
    std::destroy_at(b);
}

c4::value::value(std::unique_ptr<block, block_deleter>&& b)
    : impl{std::move(b)} { }

c4::value
c4::value::evaluate(const std::shared_ptr<evaluation_stack>& stk,
                    const std::span<const ast::expression*> args) const {
    return std::visit(evaluator_visitor(stk, args), impl);
}

c4::value
c4::value::from_block_ast(const ast::block_expression* blk_expr,
                          std::shared_ptr<evaluation_stack>&& stk) {
    return {
        std::unique_ptr<block, block_deleter>(new block(blk_expr, std::move(stk)),
                                              block_deleter())
    };
}

c4::value
c4::value::nil() {
    return {
        std::unique_ptr<block, block_deleter>(new block(nullptr),
                                              block_deleter())
    };
}

namespace {
    struct int_coercer final {
        int64_t
        operator()(const std::int64_t d) const { return d; }

        int64_t
        operator()(const double d) const { return static_cast<int64_t>(d); }

        int64_t
        operator()(const std::string& s) const {
            int64_t out;
            if (auto [ptr, errc] = std::from_chars(
                    s.data(),
                    s.data() + s.length(),
                    out);
                errc != std::errc{}) {
                return 0;
            }
            return out;
        }

        int64_t
        operator()(const c4::symbol& s) const {
            return s.arity;
        }

        int64_t
        operator()(const std::unique_ptr<c4::block, c4::block_deleter>& b) const {
            return 0;
        }
    };

    struct str_coercer final {
        template<class T>
        std::string
        operator()(const T val) const {
            return std::format("{}", val);
        }

        std::string
        operator()(const std::string& s) const {
            return s;
        }

        std::string
        operator()(const c4::symbol& s) const {
            return std::format("{}/{}", s.name, s.arity);
        }

        std::string
        operator()(const std::unique_ptr<c4::block, c4::block_deleter>& b) const {
            std::ostringstream oss;
            oss << b;
            return oss.str();
        }
    };

    struct truthy_evaluator final {
        bool
        operator()(const auto&) const { return true; }

        bool
        operator()(const std::unique_ptr<c4::block, c4::block_deleter>& b) const {
            return !b->is_nil();
        }
    };
}

std::int64_t
c4::value::coerce_to_int() const {
    return std::visit(int_coercer{}, impl);
}

std::string
c4::value::coerce_to_string() const {
    return std::visit(str_coercer{}, impl);
}

bool
c4::value::truthy() const noexcept {
    return std::visit(truthy_evaluator{}, impl);
}

std::ostream&
c4::operator<<(std::ostream& os, const value& obj) {
    return std::visit(value_printer{os}, obj.impl);
}
