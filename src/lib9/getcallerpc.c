#include <u.h>
#include <libc.h>

ulong
getcallerpc(void *firstarg)
{
	USED(firstarg);
	/* only used in qlock, but ed(1) isn't a threaded program */
	sysfatal("ed(1) needs getcallerpc!");
	return 666ul;	/* obviously not reached */
}
