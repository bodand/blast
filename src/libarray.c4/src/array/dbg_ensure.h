#ifndef dbg_ensure
#  ifdef NDEBUG
#    define dbg_ensure(exp, ...) __VA_ARGS__
#  else
#    define dbg_ensure(exp, ...) assert(__VA_ARGS__ exp)
#  endif
#endif

