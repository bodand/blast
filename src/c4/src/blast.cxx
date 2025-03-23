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
#include <fmt/std.h>

#ifdef _WIN32
#  define WIN32_LEAN_AND_MEAN
#  include <windows.h>
#endif

#include <c4/p2/parser.hxx>
#include <c4/p2/lex/lexer.hxx>
#include <mio/mmap.hpp>

using namespace std::literals;

int
main() {
#ifdef _WIN32
    SetConsoleCP(CP_UTF8);
    SetConsoleOutputCP(CP_UTF8);
#endif

    // std::error_code ec;
    // auto file = mio::make_mmap_source("file.c4", 0, mio::map_entire_file, ec);
    // std::cout << (void*)file.data() << std::endl;

    const auto buf = "# \0mao"s + R"__(
# asdasd
let (-?)/2 left 2 { |a b| b - a }
let (^) { |x| print x }
let else/1 \|x| x

let x readln

if str_empty x {
    println "Nem adtál meg szöveget"
}
else {
    println cat "A szöveged: " x
}

let default/1 { |x| { |val| x } }
let case/3 { |x code next|
    { |val|
        if x == val code
        else &(next)/1 val
    }
}
let switch/2 { |val case|
    &(case)/1 val
}
let y readln
println
    cat "Párja: "
        switch y
            case 0 10
            case 1 11
            default (add -1 y)

let printn/1 { |n|
    let printn_impl/1 { |n|
        print cat n " "
        if n == 0 {}
            printn_impl add -1 n
    }
    printn_impl n
    println ""
}
printn 24

let xsd
    if readln == 0
        let asd "a"
        let bsd "b"

println &(xsd)/0

let különben/1 \|x| x
0
)__";
    c4::p2::lexer lexer("<string>", buf.c_str(), buf.c_str() + buf.size());
    c4::p2::parser parser(std::move(lexer));

    parser.declare_binop("+", 4, false);
    parser.declare_binop("-", 4, false);
    parser.declare_binop("*", 5, false);
    parser.declare_binop("/", 5, false);
    parser.declare_binop("^", 5, true);
    parser.declare_binop("==", 3, true);

    parser.declare_binop("<<", 6, false);
    parser.declare_binop(">>", 5, true);

    parser.declare_uniop("~");

    parser.declare_symbol("print", 1);
    parser.declare_symbol("println", 1);
    parser.declare_symbol("add", 2);
    parser.declare_symbol("if", 3);
    parser.declare_symbol("str_empty", 1);
    parser.declare_symbol("cat", 2);
    parser.declare_symbol("readln", 0);

    try {
        const auto exp = parser.parse_script();

        if (!parser.valid()) return 1;
    }
    catch (c4::p2::bad_token_error const&) {
        return 1;
    }

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
}
