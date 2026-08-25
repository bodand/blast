; bridge.ll --
;   Bridges the C4 runtime to the C runtime by calling the C4 runtime from the
;   main entrypoint of libc.

%c4_completion_t = type { ptr, ptr, ptr }

declare tailcc void @_c4_main(ptr, ptr)
declare void @_c4_gc_init()

define hidden tailcc void @_c4_sink(ptr %self, ptr %value) {
    ret void
}

define i32 @main(i32 %argc, ptr %argv) {
    call void @_c4_gc_init()

    %sink = alloca %c4_completion_t, align 8

    store ptr @_c4_sink, ptr %sink, align 8

    %sink.target = getelementptr inbounds %c4_completion_t, ptr %sink, i32 0, i32 1
    store ptr null, ptr %sink.target, align 8
    %sink.K = getelementptr inbounds %c4_completion_t, ptr %sink, i32 0, i32 2
    store ptr null, ptr %sink.K, align 8

    call tailcc void @_c4_main(ptr null, ptr %sink)

    ret i32 0
}
