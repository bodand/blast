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
 * Originally created: 2025-02-02.
 *
 * src/blast --
 *   
 */

#include <iomanip>
#include <iostream>

#include <fmt/format.h>

#include <c4/block.hxx>
#include <c4/interpreter.hxx>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

int
main() {
#ifdef _WIN32
    SetConsoleCP(CP_UTF8);
    SetConsoleOutputCP(CP_UTF8);
#endif

    const std::string buf = R"__(
let else/1 \|x| x

print "Szöveg:"
let x/0 readln

if str_empty x {
    println "Nem adtál meg szöveget"
}
else {
    println cat "A szöveged: " x
}

let default/1 { |x| { |val| x } }
let case/3 { |x code next|
    { |val|
        if eq x val code
        else &next/1 val
    }
}
let switch/2 { |val case|
    &case/1 val
}

let xsd/0
    if eq 1 readln
        let asd/0 "a"
        let bsd/0 "b"

println &xsd/0

print "Szám:"
let y/0 readln
(println
    (cat "Párja: "
        (switch y
            (case 0 10
            (case 1 11
            (default (add -1 y)))))))

let printn/1 { |n|
    let printn_impl/1 { |n|
        print cat n " "
        if eq 0 n {}
            printn_impl add -1 n
    }
    printn_impl n
    println ""
}
printn 24

0
)__";


// print "Szöveg:"
// let x/0 &readln/0
//
// if str_empty x {
//     println "Nem adtál meg szöveget"
// }
// else
//     println cat "A szöveged: " x
//

//
// let printn/1 { |n|
//     let printn_impl/1 { |n|
//         if eq 0 n {} {
//             printn_impl add -1 n
//             print cat n " "
//         }
//     }
//     printn_impl n
//     println ""
// }
// printn 35
//
// print "Szöveg:"
// let x/0 &{
//     let x/0 readln
//     x
// }/0
//
// if str_empty x {
//     println "Nem adtál meg szöveget"
// }
// else
//     println cat "A szöveged: " x


    // if {} { print "yes" } { print "no" }
    //
    // let else/1 \|x| x
    // let then/1 \|x| x
    //
    // if {} then { print "yes" } else { print "no" }

    // let a/0 0
    // let inc/1 { |x|
    //     print "asd"
    //     add 1 41
    // }
    //
    // print &1/0
    //
    // print
    //     if a "true0" "false0"
    // print
    //     if a \"true0" \"false0"
    // print
    //     if a then "true1" else "false1"
    // print
    //     if a "true2" else "false2"
    // print
    //     if a { "true3" } { "false3" }
    // print
    //     if a { "true4" } else { "false4" }
    // print
    //     if a
    //     then {
    //         "true5"
    //     }
    //     else {
    //         "false5"
    //     }

    c4::interpreter interpreter;
    interpreter.define("print", 1,
                       std::unique_ptr<c4::block, c4::block_deleter>(
                           new c4::native_block([](auto& stack) -> c4::value {
                               auto val = stack->value_of(c4::symbol("$0", 0));
                               std::cout << **val;
                               return std::move(**val);
                           }))
    );
    interpreter.define("println", 1,
                       std::unique_ptr<c4::block, c4::block_deleter>(
                           new c4::native_block([](auto& stack) -> c4::value {
                               auto val = stack->value_of(c4::symbol("$0", 0));
                               std::cout << **val << "\n";
                               return std::move(**val);
                           }))
    );
    interpreter.define("readln", 0,
                       std::unique_ptr<c4::block, c4::block_deleter>(
                           new c4::native_block([](auto& stack) -> c4::value {
                               std::string w;
                               std::getline(std::cin, w);
                               return w;
                           }))
    );
    interpreter.define("str_empty", 1,
                       std::unique_ptr<c4::block, c4::block_deleter>(
                           new c4::native_block([](auto& stack) -> c4::value {
                               auto val = *stack->value_of(c4::symbol("$0", 0));
                               if (const auto str = val->coerce_to_string();
                                   str.empty())
                                   return 1;
                               return c4::value::nil();
                           }))
    );
    interpreter.define("add", 2,
                       std::unique_ptr<c4::block, c4::block_deleter>(
                           new c4::native_block([](auto& stack) -> c4::value {
                               auto val0 = *stack->value_of(c4::symbol("$0", 0));
                               auto val1 = *stack->value_of(c4::symbol("$1", 0));
                               return val0->coerce_to_int() + val1->coerce_to_int();
                           }))
    );
    interpreter.define("eq", 2,
                       std::unique_ptr<c4::block, c4::block_deleter>(
                           new c4::native_block([](auto& stack) -> c4::value {
                               auto val0 = *stack->value_of(c4::symbol("$0", 0));
                               auto val1 = *stack->value_of(c4::symbol("$1", 0));
                               const auto eq = val0->coerce_to_int() == val1->coerce_to_int();
                               if (eq) return eq;
                               return c4::value::nil();
                           }))
    );
    interpreter.define("cat", 2,
                       std::unique_ptr<c4::block, c4::block_deleter>(
                           new c4::native_block([](auto& stack) -> c4::value {
                               auto val0 = *stack->value_of(c4::symbol("$0", 0));
                               auto val1 = *stack->value_of(c4::symbol("$1", 0));
                               return val0->coerce_to_string() + val1->coerce_to_string();
                           }))
    );
    interpreter.define("if", 3,
                       std::unique_ptr<c4::block, c4::block_deleter>(
                           new c4::native_block([](auto& stack) -> c4::value {
                               if (auto cond = *stack->value_of(c4::symbol("$0", 0));
                                   cond->truthy()) {
                                   auto val0 = *stack->value_of(c4::symbol("$1", 0));
                                   return val0->evaluate(stack, {});
                               }

                               auto val1 = *stack->value_of(c4::symbol("$2", 0));
                               return val1->evaluate(stack, {});
                           }))
    );
    // interpreter.define("else", 1,
    //                    std::unique_ptr<c4::block, c4::block_deleter>(
    //                        new c4::native_block([](auto& stack) -> c4::value {
    //                            auto val0 = *stack.value_of(c4::symbol("$0", 0));
    //                            return std::move(*val0);
    //                        }))
    // );

    try {
        interpreter.parse(buf);
        return interpreter.exec();
    }
    catch (std::runtime_error& x) {
        std::cerr << "fatal: " << x.what() << "\n";
    }
}
