define i64 @_c4__arg_loader(ptr %arg) nounwind {
body:
    %"pair.0.addr" = getelementptr ptr, ptr %arg, i64 0
    %"pair.1.addr" = getelementptr ptr, ptr %arg, i64 1
    %"pair.0" = load ptr, ptr %"pair.0.addr", align 8
    %"pair.1" = load ptr, ptr %"pair.1.addr", align 8
    %"pair.0.isnull" = icmp eq ptr %"pair.0", null
    br i1 %"pair.0.isnull", label %return, label %evalbase

evalbase:
    %"pair.1.isnull" = icmp eq ptr %"pair.1", null
    br i1 %"pair.1.isnull", label %eval_noctx, label %eval_ctx

eval_noctx:
    %val0 = call i64 %"pair.0"()
    store ptr null, ptr %"pair.0.addr", align 8
    store i64 %val0, ptr %"pair.1.addr", align 8
    ret i64 %val0

eval_ctx:
    %val1 = call i64 %"pair.0"(ptr %"pair.1")
    store ptr null, ptr %"pair.0.addr", align 8
    store i64 %val1, ptr %"pair.1.addr", align 8
    ret i64 %val1

return:
    %mem = ptrtoint ptr %"pair.1" to i64
    ret i64 %mem
}

declare i64 @_Cr7println1(i64)
declare i64 @_Cr5print1(i64)

declare ptr @c4rt_datum_coerce_string(i64)
declare void @c4rt_free(ptr)
declare i64 @c4rt_datum_from_int32(i64)

define i64 @_C7println1(ptr %x) {
body:
    %x.real = call i64 @_c4__arg_loader(ptr %x)
    %0 = call i64 @_Cr7println1(i64 %x.real)
    ret i64 %0
}

define i64 @_C5print1(ptr %x) {
body:
    %x.real = call i64 @_c4__arg_loader(ptr %x)
    %0 = call i64 @_Cr5print1(i64 %x.real)
    ret i64 %0
}

define i64 @_C9str_empty1(ptr %str) {
body:
    %str.real = call i64 @_c4__arg_loader(ptr %str)
    %str.str = call ptr @c4rt_datum_coerce_string(i64 %str.real)
    %str.addr = getelementptr i8, ptr %str.str, i64 0
    %str.char0 = load i8, ptr %str.addr, align 1
    %"zero?" = icmp eq i8 %str.char0, 0
    call void @c4rt_free(ptr %str.str)
    br i1 %"zero?", label %true, label %false

true:
    %tval = call i64 @c4rt_datum_from_int32(i32 1)
    ret i64 %tval

false:
    ret i64 s0x7FF0000000000000
}

define i64 @_C2if3(ptr %cond, ptr %true, ptr %false) {
eval:
    %cond.real = call i64 @_c4__arg_loader(ptr %cond)
    %isfalse = icmp eq i64 %cond.real, s0x7FF0000000000000
    br i1 %isfalse, label %branch_f, label %branch_t

branch_f:
    %false.real = call i64 @_c4__arg_loader(ptr %false)
    ret i64 %false.real

branch_t:
    %true.real = call i64 @_c4__arg_loader(ptr %true)
    ret i64 %true.real
}

