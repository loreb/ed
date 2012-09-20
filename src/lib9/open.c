#define _GNU_SOURCE	/* for Linux O_DIRECT */
#include <u.h>
#define NOPLAN9DEFINES
#include <sys/file.h>
#include <libc.h>
#ifndef O_DIRECT
#define O_DIRECT 0
#endif

int
p9open(char *name, int mode)
{
	int cexec, rclose;
	int fd, umode, lock, rdwr;

	rdwr = mode&3;
	umode = rdwr;
	cexec = mode&OCEXEC;
	rclose = mode&ORCLOSE;
	lock = mode&OLOCK;
	mode &= ~(3|OCEXEC|ORCLOSE|OLOCK);
	if(mode&OTRUNC){
		umode |= O_TRUNC;
		mode ^= OTRUNC;
	}
	if(mode&ODIRECT){
		umode |= O_DIRECT;
		mode ^= ODIRECT;
	}
	if(mode&ONONBLOCK){
		/* windows doesn't have/need it */
		sysfatal("umode |= O_NONBLOCK");
		mode ^= ONONBLOCK;
	}
	if(mode&OAPPEND){
		umode |= O_APPEND;
		mode ^= OAPPEND;
	}
	if(mode){
		werrstr("mode 0x%x not supported", mode);
		return -1;
	}
	fd = open(name, umode);
	if(fd >= 0){
		/* see create.c */
		if(lock)
			sysfatal("open(OLOCK)");
		if(cexec)
			sysfatal("open(OCEXEC)");
		if(rclose)
			remove(name);
	}
	return fd;
}
