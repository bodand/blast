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
 * src/c4rt2/src/c4rt2/c4rt_package --
 *   
 */

#include <assert.h>
#include <c4rt2/c4rt.h>

#include "c4rt_package_versions.h"
#include "package.1.h"

C4RT_API c4_ptr64_t
c4rt_pad_pointer(void* const ptr) {
    return c4rt_pad_pointer_1(ptr);
}

C4RT_API void*
c4rt_unpad_pointer(const c4_ptr64_t ptr) {
    return c4rt_unpad_pointer_1(ptr);
}

void
c4rt_package_init(struct c4_package_t* const pkg) {
    assert(pkg && "pkg must not be null");
    c4rt_package_init_1(pkg);
}

void
c4rt_package_set_from_packages(struct c4_package_t* const pkg,
                               c4rt_package_function_t* const calc_fun,
                               struct c4_package_t* const fn_data) {
    assert(pkg && "pkg must not be null");
    assert(calc_fun && "calc_fun must not be null");
    c4rt_package_set_from_packages_1(pkg, calc_fun, fn_data);
}

void
c4rt_package_set_from_result(struct c4_package_t* const pkg,
                             const c4_datum_t datum) {
    assert(pkg && "pkg must not be null");
    c4rt_package_set_from_result_1(pkg, datum);
}

C4RT_API c4_datum_t
c4rt_evaluate_package(struct c4_package_t* const pkg) {
    assert(pkg && "pkg must not be null");
    return c4rt_evaluate_package_1(pkg);
}
