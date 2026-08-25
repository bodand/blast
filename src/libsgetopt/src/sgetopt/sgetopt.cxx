/* ISC license. */

/* MT-unsafe */

#include <format>

#undef SUBGETOPT_SHORT

#include <sgetopt/sgetopt.h>

int
sgetopt_r(int argc, char const *const *argv, char const *opts, subgetopt *o)
{
	int c = subgetopt_r(argc, argv, opts, o);
	
	if (o->err && ((c == '?') || (c == ':')))
	{
		std::fputs(std::format("{}: {}\n",
					o->prog ? o->prog : argv[0],
					((c == '?') && argv[o->ind] && (o->ind < argc)) 
						? "illegal option" 
						: "option requires an argument").c_str(),
				stderr);
	}

	return c;
}
