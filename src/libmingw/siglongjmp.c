#define NOPLAN9DEFINES
#include <u.h>
#include <libc.h>

void siglongjmp(sigjmp_buf env, int val)
{
	fprint(2, "todo: siglongjmp(env, %d)\n", val);
	longjmp(env->buf, val);
}
