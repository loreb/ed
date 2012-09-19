#include <u.h>
#define NOPLAN9DEFINES
#include <libc.h>

extern void p9main(int, char**);

int
main(int argc, char **argv)
{
	/* Binary mode on standard descriptors -- now! */
	int fd;
	for(fd = 0; fd <= 2; fd++)
		if(setmode(fd, O_BINARY) < 0)
			sysfatal("binmode(%d): %r", fd);
	p9main(argc, argv);
	exits("main");
	return 99;
}
