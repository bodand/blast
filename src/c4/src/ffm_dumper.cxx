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
 * src/c4/src/ffm_dumper --
 *   
 */

#include <c4/ffm_dumper.hxx>

#include <c4/ffm/function.hxx>
#include <c4/ffm/function_call.hxx>
#include <c4/ffm/function_declaration.hxx>
#include <c4/ffm/function_definition.hxx>
#include <c4/ffm/function_pack.hxx>
#include <c4/ffm/symbol.hxx>

void
c4::ffm_dumper::do_visit(const ffm::function& obj) {
    obj.accept_skip_self(*this);
}

void
c4::ffm_dumper::print_call_like(const std::string_view call_type,
                                const ffm::function_declaration* obj,
                                const std::span<ffm::value_expression* const> args) {
    _os << call_type << " " << obj->name() << "( ";
    for (const auto& arg : args) {
        const auto last = _call_end;
        _call_end = ' ';
        arg->accept_skip_self(*this);
        _call_end = last;
    }
    _os << ")" << _call_end;
}

void
c4::ffm_dumper::do_visit(const ffm::function_call& obj) {
    print_call_like("call", obj.function(), obj.arguments());

}

void
c4::ffm_dumper::do_visit(const ffm::function_pack& obj) {
    print_call_like("pack", obj.function(), obj.arguments());
}

void
c4::ffm_dumper::do_visit(const ffm::function_declaration& obj) {
    if (obj.ctx_type()) {
        _os << "type " << obj.ctx_type()->name() << "( ";
        for (const auto& ref : obj.ctx_type()->fields()) _os << ref->name() << " ";
        _os << ")\n\n";
    }

    _os << "decl " << obj.name() << "( ";
    for (const auto& ref : obj.arguments()) _os << ref->name() << " ";
    _os << ")\n";
}

void
c4::ffm_dumper::do_visit(const ffm::function_definition& obj) {
    _os << "defn " << obj.name() << "( ";
    for (const auto& ref : obj.decl()->arguments()) _os << ref->name() << " ";
    _os << ") {\n";

    for (const auto& expr : obj.body()) {
        _os << "\t";
        expr->accept_skip_self(*this);
    }
    _os << "}\n";
}
