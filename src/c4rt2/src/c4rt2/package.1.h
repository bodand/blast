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
#ifndef BLAST_C4RT_PACKAGE_1_H
#define BLAST_C4RT_PACKAGE_1_H

C4RT_IMPL c4_ptr64_t
c4rt_pad_pointer_1(void* ptr);

C4RT_IMPL void*
c4rt_unpad_pointer_1(c4_ptr64_t ptr);

C4RT_IMPL void
c4rt_package_init_1(struct c4_package_t* pkg);

C4RT_IMPL void
c4rt_package_set_from_function_1(struct c4_package_t* pkg,
                                 c4rt_package_function_t* calc_fun,
                                 struct c4_package_t* fn_data,
                                 uint16_t fn_data_sz);

C4RT_IMPL void
c4rt_package_set_from_result_1(struct c4_package_t* pkg,
                               c4_datum_t datum);

C4RT_IMPL c4_datum_t
c4rt_package_evaluate_1(struct c4_package_t* pkg);

#endif
