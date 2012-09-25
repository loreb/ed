#include <u.h>
#include <libc.h>

void _sigjmp_savemask(sigjmp_buf env, int savemask)
{
	env->savemask = savemask;
}

void _sigjmp_restoremask(sigjmp_buf env)
{
	if(env->savemask)
		fprint(2, "%s todo restoremask\n", __FILE__);
}
