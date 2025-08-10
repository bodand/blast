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
 * Originally created: 2025-08-08.
 *
 * src/c4/include/c4/ffm_mapper --
 *   
 */
#ifndef BLAST_FFM_MAPPER_HXX
#define BLAST_FFM_MAPPER_HXX

#include <optional>
#include <span>
#include <vector>

#include <c4/ffm/ffm_context.hxx>
#include <c4/visitor/visitor.hxx>
#include <fmt/ranges.h>

namespace c4::ast2 {
    struct symbol;
    struct block;
    struct block_args;
    struct let_expression;
    struct expression;
}

namespace c4 {
    template<class T>
    struct recursive_scope {
        explicit
        recursive_scope(T& counter, T temp = T()) noexcept
            : _original_value(counter)
            , _ref(counter) {
            counter = temp;
        }

        ~recursive_scope() noexcept { _ref = _original_value; }

    private:
        T _original_value;
        T& _ref;
    };

    template<class T>
    recursive_scope(T&) -> recursive_scope<T>;

    struct ffm_mapper final : ast2::visitor<
                ast2::block,
                ast2::block_args,
                ast2::expression,
                ast2::let_expression
            > {
        explicit ffm_mapper(ffm::ffm_context& ffm_context);

        void do_visit(const ast2::let_expression& obj) override;

        void do_visit(const ast2::block& obj) override;

        void do_visit(const ast2::block_args& obj) override;

        void do_visit(const ast2::expression& obj) override;

        [[nodiscard]] std::span<ffm::function* const>
        roots() const noexcept { return _roots; }

    private:
        struct block_entry {
            explicit
            block_entry(std::size_t& counter) noexcept
                : _block_id(counter)
                , _counter(counter) {
                counter = 0;
            }

            ~block_entry() noexcept { _counter = _block_id; }

        private:
            std::size_t _block_id;
            std::size_t& _counter;
        };

        [[nodiscard]] ffm::function_declaration*
        find_function_declaration(const ffm::symbol& sym) const;

        void
        declare_function(const ffm::symbol&);

        [[nodiscard("store as automatic variable")]] recursive_scope<ffm::function_definition*>
        define_function(const ffm::symbol&);

        [[nodiscard]] std::size_t
        get_next_block_id() { return _block_counter++; }

        [[nodiscard("store as automatic variable")]] recursive_scope<std::size_t>
        enter_block() noexcept { return recursive_scope(_block_counter); }

        [[nodiscard]] std::string
        mangled_scope(const std::string_view mangled) const {
            if (_block_names.empty()) return std::string(mangled);
            return fmt::format("N{}E{}", fmt::join(_block_names, ""), mangled);
        }

        [[nodiscard]] std::string
        next_block_name(unsigned arity);

        std::size_t _block_counter{0};
        std::vector<std::string_view> _block_names{};
        bool _skip_implicit_block_entry{false};
        bool _implicit_block_entry_closure{false};

        ffm::function_definition* _current_function{};

        std::vector<ffm::function*> _roots{};
        ffm::ffm_context& _ffm_context;
    };
}

#endif
