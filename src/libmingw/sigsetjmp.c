#define NOPLAN9DEFINES 1
#include <u.h>
#include <libc.h>

int sigsetjmp(sigjmp_buf env, int savemask)
{
	if(savemask)
		fprint(2, "todo sigsetjmp(env, %d)\n", savemask);
	return setjmp(env->buf);
}
