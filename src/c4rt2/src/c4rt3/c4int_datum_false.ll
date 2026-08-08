declare tailcc ptr @_c4_allocate_array(i64, i64) #0
declare tailcc void @_c4_apply(ptr noundef align 8, ptr noundef align 8) #1
declare tailcc void @_c4_evaluate(ptr noundef align 8, ptr noundef align 8) #1
declare tailcc ptr @_c4_make_datum_block(ptr noundef align 8) #1
declare tailcc ptr @_c4_make_thunk(ptr noundef align 8) #1
declare tailcc void @_c4_set_thunk_args(ptr, ptr, i32) #1

define ptr @c4int_datum_false() #2 {
  %1 = call tailcc ptr @_c4_make_datum_block(ptr @raw_false)
  %2 = call tailcc ptr @_c4_allocate_array(i64 2, i64 8)
  call tailcc void @_c4_set_thunk_args(ptr %1, ptr %2, i32 2)
  ret ptr %1
}

define private tailcc void @raw_false(ptr noundef align 8 %argv,
                                      ptr noundef align 8 %K) #1 {
entry:
  %1 = getelementptr ptr, ptr %argv, i64 1
  %f = load ptr, ptr %1, align 8
  musttail call tailcc void @_c4_evaluate(ptr %f, ptr %K)
  ret void
}

attributes #0 = { nounwind willreturn allockind("alloc,zeroed") allocsize(1,0) }
attributes #1 = { nofree nounwind }
attributes #2 = { nounwind willreturn }