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

#ifndef ALIGNAS
#  if defined(_MSC_VER) && !defined(__clang__)
#    define ALIGNAS(x)
#  else
#    define ALIGNAS(x) __attribute__((aligned(x)))
#  endif
#endif

/**
 * ABI stable package struct for v1.
 *
 * A package can be in any of the states described below. The \c package_size
 * member always holds the length of the structure in memory. Other than that,
 * every member not mentioned has undefined values.
 *
 * 1. Completed
 *      This package has been calculated, no further function calls
 *      are required. In this state:
 *      - \c completed is \c true
 *      - \c data is a single datum embedded in the value
 * 2. Incomplete nullary function:
 *      The package awaits evaluation using a nullary function. In this state:
 *      - \c completed is \c false
 *      - \c function_arity is \c 0
 *      - \c dynamic_chain is \c false
 *      - \c data is a single embedded pointer pointing to the callee
 * 3. Incomplete nullary datum:
 *      The package awaits evaluation using a nullary datum object embedded. In
 *      this state:
 *      - \c completed is \c false
 *      - \c function_arity is \c 0
 *      - \c dynamic_chain is \c true
 *      - \c data is a single embedded datum object pointing to the callee
 * 3. Incomplete non-closure:
 *      The package awaits evaluation using a non-closure function that takes
 *      normal parameters. In this state:
 *      - \c completed is \c false
 *      - \c function_arity is some N > 0
 *      - \c context_sz_divided_bytes is \c 0
 *      - \c dynamic_chain is \c false
 *      - \c data is a pointer to a c4_package_v1_function_payload object
 *          of size sizeof(void(*)()) + \c function_arity * \c package_size
 *
 * 4. Incomplete closure:
 *      The package awaits evaluation using a closure that may or may not take
 *      other parameters. In this state:
 *      - \c completed is \c false
 *      - \c function_arity is some N > 0
 *      - \c context_sz_divided_bytes is some C > 0
 *      - \c data_sz_bytes is a pointer to a c4_package_v1_function_payload object
 *          of size sizeof(void(*)()) + (\c context_sz_divided_bytes +
 *          \c function_arity) * \c package_size
 */
struct c4_package_v1_t {
    /// Specifies the size of this package: used to differentiate the ABI
    /// version used.
    uint16_t version;
    /// The callee functions's base arity, including the optional closure
    /// context parameter.
    uint16_t function_arity;
    /// Whether this package has been completed. If yes, data contains a
    /// single datum, otherwise an encoded function payload.
    uint8_t completed;
    /// Whether this package packages a datum object to be evaluated with
    /// c4rt_datum_evaluate instead of a proper function.
    uint8_t dynamic_datum : 4;
    /// Whether this package is to be chainloaded. That is, data contains a
    /// c4_package_v1_chainload_payload pointer. Chainload evaluation will be
    /// used.
    uint8_t chainload : 4;
    /// The size of the first parameter in bytes rounded up to the next multiple
    /// of 8, then divided by 8.
    /// This is used to specify the special context parameters of closures.
    /// If not holding a closure, it is zero, see the struct's documentation for
    /// a richer explanation of possible states.
    uint16_t context_sz_divided_bytes;
    /// Either an embedded value or a pointer to a c4_package_v1_function_payload
    /// object that is dynamically allocated.
    c4_ptr64_t data;
};

/**
 * Payload that stores a function pointer with the required place for parameters
 * to be called with. The parameters are to be copied into args.
 *
 * Closures:
 *      Closures have a special zeroth parameter that is their context.
 *      These are placed at the front of args, with their sizes rounded up in
 *      a way that each context takes a multiple of package sizes.
 */
struct c4_package_v1_function_payload {
    c4rt_package_function_t* calc_fun;
    ALIGNAS(8) char args_untyped[];
};

struct c4_package_v1_chainload_payload {
    c4rt_package_function_t* calc_fun;
    char* args_untyped_start;
    ALIGNAS(8) char raw[];
};

#define C4PKGV1_CHAINLOAD_ARGS(payload) \
    ( (void*)payload->args_untyped_start )

/**
 * c4rt_package_init_from_function_v1(*pkg, *calc_fun,
 *                                    *fn_data, fn_data_sz_bytes) --
 *   Creates a yet to be calculated lazy package into *pkg, memory is allocated
 *   dynamically as needed. Ensure that c4rt_package_free(pkg) is called on
 *   it, or risk leaking memory.
 *
 *   Preconditions:
 *   - The pkg value must not be NULL.
 *   - If fn_data is NULL, fn_data_sz must be zero.
 *   - If fn_data_sz is zero, fn_data may be any value.
 *   - The pointer pointing to fn_data must point to an array at least as large
 *   as to hold fn_data_sz elements.
 *   - The calc_fun value must not be NULL.
 */
C4RT_IMPL void
c4rt_package_init_from_function_v1(struct c4_package_v1_t* pkg,
                                   c4rt_package_function_t* calc_fun,
                                   const struct c4_package_v1_t** fn_data,
                                   uint16_t fn_data_sz_bytes);

/**
 * c4rt_package_init_from_closure_v1(*pkg, *calc_fun,
 *                                   *ctx, ctx_sz,
 *                                   *fn_data, fn_data_sz_bytes) --
 *   Creates a yet to be calculated lazy package into *pkg, memory is allocated
 *   dynamically as needed. Ensure that c4rt_package_free(pkg) is called on
 *   it, or risk leaking memory.
 *   The context object at [ctx, ctx_sz) will be copied into the package
 *   shallowly so ensure each dynamic object is not released until this package
 *   exists.
 *
 *   Preconditions:
 *   - The pkg value must not be NULL.
 *   - If fn_data is NULL, fn_data_sz must be zero.
 *   - If fn_data_sz is zero, fn_data may be any value.
 *   - The pointer pointing to fn_data must point to an array at least as large
 *   as to hold fn_data_sz elements.
 *   - The calc_fun value must not be NULL.
 *   - The ctx value must not be NULL.
 *   - The ctx_sz value must be > 0.
 */
C4RT_IMPL void
c4rt_package_init_from_closure_v1(struct c4_package_v1_t* pkg,
                                  c4rt_package_function_t* calc_fun,
                                  const void* ctx,
                                  uint32_t ctx_sz_bytes,
                                  const struct c4_package_v1_t** fn_data,
                                  uint32_t fn_data_sz_bytes);

/**
 * c4rt_package_init_from_dynamic_v1(*pkg, datum, *fn_data, fn_data_sz_bytes) --
 *   Creates a package from a completed calculation's datum. Does not allocate
 *   if fn_data_sz_bytes is zero.
 *
 *   Preconditions:
 *   - The pkg value must not be NULL.
 *   - If fn_data is NULL, fn_data_sz must be zero.
 */
C4RT_IMPL void
c4rt_package_init_from_dynamic_v1(struct c4_package_v1_t* pkg,
                                  c4_datum_t datum,
                                  const struct c4_package_v1_t** fn_data,
                                  uint32_t fn_data_sz_bytes);

/**
 * c4rt_package_init_from_result_v1(*pkg, datum) --
 *   Creates a package from a completed calculation's datum. Never allocates.
 *
 *   Preconditions:
 *   - The pkg value must not be NULL.
 */
C4RT_IMPL void
c4rt_package_init_from_result_v1(struct c4_package_v1_t* pkg,
                                 c4_datum_t datum);

/**
 * c4rt_package_evaluate_v1(*pkg) --
 *   Evaluates a given package. Blocks if not yet completed, and calls the
 *   stored function. The storage for the function callee is released in this
 *   case and the result is embedded as if by c4rt_package_init_from_result.
 *
 *   Returns internal datum if already completed.
 *
 *   Preconditions:
 *   - The pkg value must not be NULL.
 */
C4RT_IMPL c4_datum_t
c4rt_package_evaluate_v1(void* pkg);

/**
 * c4rt_package_free(*pkg) --
 *   Releases memory allocated (if any) to hold the internal state of the
 *   package. Note: the passed pkg pointer itself is not freed.
 *
 *   Preconditions:
 *   - The pkg value must not be NULL.
 */
C4RT_IMPL void
c4rt_package_free_v1(const struct c4_package_v1_t* pkg);

#endif
