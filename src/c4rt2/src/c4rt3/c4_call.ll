; c4_call.ll --
;   Provides a C API to call into a C4 defined datum. It returns the evaluated
;   datum for further use. Note that it could be any C4 datum, not necessarily
;   evaluated.

%c4_completion_t = type { ptr, ptr, ptr }

declare tailcc void @_c4_evaluate(ptr noundef align 8, ptr noundef align 8) #1

define hidden tailcc void @_c4_returner(ptr %self, ptr %value) {
    %ret.target = getelementptr inbounds %c4_completion_t, ptr %self, i32 0, i32 1
    store ptr %value, ptr %ret.target

    ret void
}

define ptr @c4_call(ptr %callee) {
    %ret = alloca %c4_completion_t, align 8

    store ptr @_c4_returner, ptr %ret, align 8

    %ret.target = getelementptr inbounds %c4_completion_t, ptr %ret, i32 0, i32 1
    store ptr null, ptr %ret.target, align 8
    %ret.K = getelementptr inbounds %c4_completion_t, ptr %ret, i32 0, i32 2
    store ptr null, ptr %ret.K, align 8

    call tailcc void @_c4_evaluate(ptr %callee, ptr %ret)

    %res = load ptr, ptr %ret.target, align 8
    ret ptr %res
}

attributes #1 = { nofree nounwind }
