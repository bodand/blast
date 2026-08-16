; c4_apply.ll --
;   Provides a C API to apply args onto a C4 defined datum. It returns the
;   result datum. Ensure not to call with insufficient arguments.

%c4_completion_t = type { ptr, ptr, ptr }

declare tailcc ptr @_c4_make_thunk(ptr noundef align 8) #1
declare tailcc void @_c4_apply(ptr noundef align 8, ptr noundef align 8) #1
declare tailcc ptr @_c4_allocate_array(i64, i64) #2
declare noalias ptr @GC_malloc(i64) #3

declare void @llvm.memcpy.p0.p0.i64(ptr noalias writeonly captures(none), ptr noalias readonly captures(none), i64, i1 immarg) #2

; !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
; !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
; !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
; THIS ONLY WORKS BECAUSE THE HARDWARE IS LENIENT TO BREAKING LLVM'S GUARANTEES
; BECAUSE IT INDIRECTLY JUMPS INTO THIS FUNCTION BELIEVING IT TO BE TAILCC WHICH
; IT IS NOT. IF IT WERE, IT WOULD TRY TO CALLEE POP THE STACK AND THEN RET TO
; SOME RANDOM SHIT IT FOUND IN MEMORY BECAUSE IT POPPED THE RETURN ADDRESS...
; THIS BEING CDECL (CALLER CLEANUP) MEANS IT DOESN'T POP SHIT AND RETURNS
; CORRECTLY.
; !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
; !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
; !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
define hidden void @_c4_returner(ptr %self, ptr %value) {
    %self.payload = getelementptr inbounds %c4_completion_t, ptr %self, i32 0, i32 1
    store ptr %value, ptr %self.payload                                         ; %self.payload = %value

    ret void
}

define ptr @c4_apply(ptr noundef %callee, i32 %n, ptr noundef %args, i64 %args_sz) {
    %ret_ptr = call ptr @GC_malloc(i64 24)                                      ; c4_completion_t ret_ptr = GC_malloc(8 + 8 + 8)

    store ptr @_c4_returner, ptr %ret_ptr, align 8                              ; ret.self = @_c4_returner

    %ret.payload = getelementptr inbounds %c4_completion_t, ptr %ret_ptr, i32 0, i32 1
    store ptr null, ptr %ret.payload, align 8                                   ; ret.target = null

    %ret.K = getelementptr inbounds %c4_completion_t, ptr %ret_ptr, i32 0, i32 2
    store ptr null, ptr %ret.K, align 8                                         ; ret.K = null

    %n64 = zext i32 %n to i64                                                   ; n64 = (uint64)n

    %n_inc = add nsw nuw i64 %n64, 1
    %argv = call ptr @_c4_allocate_array(i64 %n_inc, i64 8)                     ; argv = _c4_allocate_array(%n64 + 1, 8)

    store ptr %callee, ptr %argv, align 8                                       ; argv[0] = %callee

    %argv_tail = getelementptr inbounds ptr, ptr %argv, i64 1
    %args_sz_bytes = mul nsw nuw i64 %args_sz, 8
    call void @llvm.memcpy.p0.p0.i64(ptr %argv_tail, ptr %args, i64 %args_sz_bytes, i1 false)
                                                                                ; memcpy(&argv[1], args, args_sz * 8)

    notail call tailcc void @_c4_apply(ptr %argv, ptr %ret_ptr)

    %res = load ptr, ptr %ret.payload, align 8
    ret ptr %res
}

attributes #0 = { nounwind willreturn allockind("alloc,zeroed") allocsize(1,0) }
attributes #1 = { nofree nounwind }
attributes #2 = { nocallback nofree nounwind willreturn memory(argmem: readwrite) }
attributes #3 = { noinline nounwind willreturn allockind("alloc,zeroed") allocsize(0) }
