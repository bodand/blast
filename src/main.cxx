#include <iostream>
#include <algorithm>
#include <iomanip>

#include "parser/ast.hxx"
#include "parser/ast_adapted.hxx"
#include "parser/parser.hxx"
#include "parser/config.hxx"

struct expr_printer {
    using result_type = void;

    void
    operator()(const std::string& str) const {
        std::cout << std::quoted(str);
    }

    void
    operator()(const demo::ast::symbol_expression& symbol) const {
        std::cout << "(" << symbol.symbol;
        if (!symbol.arguments.empty()) std::cout << " ";
        for (const auto& arg: symbol.arguments) {
            boost::apply_visitor(*this, arg);
            std::cout << " ";
        }
        if (!symbol.arguments.empty()) std::cout << "\b";
        std::cout << ")";
    }

    void
    operator()(const demo::ast::let_expression& let) const {
        std::cout << "(let " << let.symbol << "\n  ";
        boost::apply_visitor(*this, let.expr);
        std::cout << ")\n";
    }

    void
    operator()(const demo::ast::block_expression& block) const {
        std::cout << "'(";
        if (!block.parameters.parameters.empty()) {
            std::cout << "[";
            for (const auto& item: block.parameters.parameters) {
                std::cout << item.name << "/" << item.arity << " ";
            }
            std::cout << "\b] ";
        }
        for (const auto& expr: block.expressions) {
            boost::apply_visitor(*this, expr);
            std::cout << " ";
        }
        std::cout << "\b)";
    }
};

int main() {
    namespace ast = demo::ast;
    using boost::spirit::x3::ascii::space;
    using demo::script;

    demo::ast::script ret;
    std::string buf = R"__(
let else { |x/0| "" }
let x "asd"
let y {
    print x
}
let xsd if x { y } else "bsd"
)__";
    auto begin = buf.cbegin();
    auto end = buf.cend();

    using boost::spirit::x3::with;

    demo::parser::position_cache pos{begin, end};
    demo::parser::error_handler eh(begin, end, std::cerr);
    ast::symbol_table symbols{};
    auto parser =
            with<ast::position_cache_tag>(std::ref(pos))[
                    with<boost::spirit::x3::error_handler_tag>(std::ref(eh))[
                            with<ast::symbol_table_tag>(std::ref(symbols))[
                                    script()
                            ]
                    ]
            ];

    symbols.add_symbol("print", 1);
    symbols.add_symbol("if", 3);
    try {
        bool r = phrase_parse(begin, end, parser, space, ret);

        if (r && begin == end) {
            std::cout << boost::fusion::tuple_open('[');
            std::cout << boost::fusion::tuple_close(']');
            std::cout << boost::fusion::tuple_delimiter(", ");

            std::cout << "-------------------------\n";
            std::cout << "Parsing succeeded: \n";
            for (const auto& [_, sym]: symbols.symbols) {
                std::cout << sym << "\n";
            }
            for (const auto& expr: ret.expressions) {
                boost::apply_visitor(expr_printer{}, expr);
            }
            std::cout << "\n-------------------------\n";
        } else {
            std::cout << "-------------------------\n";
            std::cout << "Parsing failed\n";
            std::cout << "-------------------------\n";
        }
    } catch (std::runtime_error& x) {
        std::cerr << x.what() << "\n";
    }
}
