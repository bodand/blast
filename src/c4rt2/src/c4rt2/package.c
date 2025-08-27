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
#include <c4rt2/package.h>
#include <c4rt2/c4rt_package_versions.h>

#include "package.1.h"

C4RT_API void
c4rt_package_init_from_function(struct c4_package_t* pkg,
                                c4rt_package_function_t* calc_fun,
                                const struct c4_package_t* fn_data,
                                const uint16_t fn_data_sz) {
    assert(pkg && "pkg must not be null");
    assert(calc_fun && "calc_fun must not be null");
    assert((fn_data_sz == 0 || fn_data) && "fn_data must be null or valid");
    switch (pkg->version) {
    case C4_PACKAGE_VERSION_1://
        c4rt_package_init_from_function_v1((struct c4_package_v1_t*)pkg, calc_fun, fn_data,
                                           fn_data_sz);
        break;
    default:
        assert(false && "invalid package version");
    }
}

C4RT_API void
c4rt_package_init_from_closure(struct c4_package_t* const pkg,
                               c4rt_package_function_t* const calc_fun,
                               const void* const ctx,
                               const uint32_t ctx_sz,
                               const struct c4_package_t* fn_data,
                               const uint16_t fn_data_sz) {
    assert(pkg && "pkg must not be null");
    assert(calc_fun && "calc_fun must not be null");
    assert(ctx && "ctx must not be null");
    assert(ctx_sz > 0 && "ctx_sz must be > 0");
    assert((fn_data_sz == 0 || fn_data) && "fn_data must be null or valid");
    switch (pkg->version) {
    case C4_PACKAGE_VERSION_1://
        c4rt_package_init_from_closure_v1((struct c4_package_v1_t*)pkg,
                                          calc_fun,
                                          ctx, ctx_sz,
                                          fn_data, fn_data_sz);
        break;
    default:
        assert(false && "invalid package version");
    }
}

C4RT_API c4_datum_t
c4rt_package_evaluate(struct c4_package_t* const pkg) {
    assert(pkg && "pkg must not be null");

    switch (pkg->version) {
    case C4_PACKAGE_VERSION_1://
        return c4rt_package_evaluate_v1((struct c4_package_v1_t*)pkg);
    default:
        assert(false && "invalid package version");
        return gC4_Empty_Block;
    }
}
