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
 *   Implementation of the function versions.
 */

#include <assert.h>
#include <limits.h>

#include <c4rt2/c4rt.h>

#include "c4rt_package_versions.h"
#include "package.1.h"

C4RT_IMPL c4_ptr64_t
c4rt_pad_pointer_1(void* const ptr) {
    return ((uintptr_t) ptr) << (CHAR_BIT * (sizeof(c4_ptr64_t) - sizeof(void*)));
}

C4RT_IMPL void*
c4rt_unpad_pointer_1(const c4_ptr64_t ptr) {
    return (void*) (ptr >> (CHAR_BIT * (sizeof(c4_ptr64_t) - sizeof(void*))));
}

static void
ptr64_set_nullptr(c4_ptr64_t* ptr) {
    // nullptr may not be bitwise zero, take proper pointer and manually pad
    // it to integer to ensure proper nullptr encoding on weird platforms
    *ptr = c4rt_pad_pointer_1(NULL);
}

C4RT_IMPL void
c4rt_package_init_1(struct c4_package_t* pkg) {
    assert(pkg && "pkg must not be null");
    pkg->package_size = C4_PACKAGE_SIZE_VERSION_1;
    ptr64_set_nullptr(&pkg->function);
    pkg->result = gC4_Empty_Block;
}

C4RT_IMPL void
c4rt_package_set_from_function_1(struct c4_package_t* pkg,
                                 c4rt_package_function_t* calc_fun,
                                 struct c4_package_t* fn_data,
                                 const uint16_t fn_data_sz) {
    assert(pkg && "pkg must not be null");
    assert(calc_fun && "calc_fun must not be null");
    assert((pkg->package_size >= C4_PACKAGE_SIZE_VERSION_1)
        && "invalid package size information: only version 1 and up are supported");

    pkg->function = c4rt_pad_pointer_1(calc_fun);
    pkg->function_arity = fn_data_sz;
    pkg->data = c4rt_pad_pointer_1(fn_data);
}

C4RT_IMPL void
c4rt_package_set_from_result_1(struct c4_package_t* pkg,
                               const c4_datum_t datum) {
    assert(pkg && "pkg must not be null");
    assert((pkg->package_size >= C4_PACKAGE_SIZE_VERSION_1)
        && "invalid package size information: only version 1 and up are supported");

    ptr64_set_nullptr(&pkg->function);
    pkg->result = datum;
}

#include "dynamic_call_hacks.h"

C4RT_IMPL c4_datum_t
c4rt_package_evaluate_1(struct c4_package_t* const pkg) {
    assert(pkg && "pkg must not be null");
    assert((pkg->package_size >= C4_PACKAGE_SIZE_VERSION_1)
        && "invalid package size information: only version 1 and up are supported");

    c4rt_package_function_t* const calc_func = c4rt_unpad_pointer_1(pkg->function);
    if (calc_func) {
        struct c4_package_t* fn_data = c4rt_unpad_pointer_1(pkg->data);
        const c4_datum_t result = c4_dynamic_call(pkg->function_arity, calc_func, fn_data);

        ptr64_set_nullptr(&pkg->function);
        pkg->result = result;
    }

    return pkg->result;
}
