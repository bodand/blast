; bridge.ll --
;   Bridges the C4 runtime to the C runtime by calling the C4 runtime from the
;   main entrypoint of libc.

%c4_completion_t = type { ptr, ptr, ptr }

declare tailcc void @_c4_main(ptr, ptr)

define hidden tailcc void @_c4_sink(ptr %self, ptr %value) {
    ret void
}

define i32 @main(i32 %argc, ptr %argv) {
    %sink = alloca %c4_completion_t, align 8

    store ptr @_c4_sink, ptr %sink, align 8

    %sink.target = getelementptr inbounds %c4_completion_t, ptr %sink, i32 0, i32 1
    store ptr null, ptr %sink.target, align 8
    %sink.K = getelementptr inbounds %c4_completion_t, ptr %sink, i32 0, i32 2
    store ptr null, ptr %sink.K, align 8

    call tailcc void @_c4_main(ptr null, ptr %sink)

    ret i32 0
}

;>-----< WIP printer to test shit >-----<
declare i32 @puts(ptr)
declare i64 @write(i32, ptr, i64)

declare void @llvm.trap() #0

@yee = private unnamed_addr constant [10 x i8] c"yer fucked", align 1
@nl = private unnamed_addr constant [1 x i8] c"\0A", align 1

%c4_datum_t = type { i32, i32, ptr, ptr }
define tailcc void @q1Ss5Nprint1EE(ptr %argv, ptr %K) {
    %datum = load ptr, ptr %argv, align 8
    %argv.type = load i32, ptr %datum, align 8

    %is_string = icmp eq i32 %argv.type, 4
    br i1 %is_string, label %good, label %fucked

good:
    %argv.value.addr = getelementptr inbounds %c4_datum_t, ptr %datum, i32 0, i32 2
    %argv.value = load ptr, ptr %argv.value.addr, align 8
    call i32 @puts(ptr %argv.value)

    %cont_fn = load ptr, ptr %K, align 8
    musttail call tailcc void %cont_fn(ptr %K, ptr %datum)
    ret void

fucked:
    call i64 @write(i32 2, ptr @yee, i64 10)
    call i64 @write(i32 2, ptr @nl, i64 1)
    call i64 @write(i32 2, ptr %datum, i64 4)
    call i64 @write(i32 2, ptr @nl, i64 1)
    call void @llvm.trap()
    unreachable
}

attributes #0 = { cold noreturn nounwind memory(inaccessiblemem: write) }
