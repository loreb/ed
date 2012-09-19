#define _GNU_SOURCE	/* for Linux O_DIRECT */
#include <u.h>
#define NOPLAN9DEFINES
#include <sys/file.h>
#include <unistd.h>
#include <fcntl.h>
#include <libc.h>
#include <sys/stat.h>
#ifndef O_DIRECT
#define O_DIRECT 0
#endif

int
p9create(char *path, int mode, ulong perm)
{
	int fd, cexec, umode, rclose, lock, rdwr;

	rdwr = mode&3;
	lock = mode&OLOCK;
	cexec = mode&OCEXEC;
	rclose = mode&ORCLOSE;
	mode &= ~(ORCLOSE|OCEXEC|OLOCK);

	/* XXX should get mode mask right? */
	fd = -1;
	if(perm&DMDIR){
		if(mode != OREAD){
			werrstr("bad mode in directory create");
			goto out;
		}
		/* win32: perms need CreateDirectory */
		if(mkdir(path/*, perm&0777*/) < 0)
			goto out;
		fd = open(path, O_RDONLY);
	}else{
		umode = (mode&3)|O_CREAT|O_TRUNC | O_BINARY;
		mode &= ~(3|OTRUNC);
		if(mode&ODIRECT){
			umode |= O_DIRECT;
			mode &= ~ODIRECT;
		}
		if(mode&OEXCL){
			umode |= O_EXCL;
			mode &= ~OEXCL;
		}
		if(mode&OAPPEND){
			umode |= O_APPEND;
			mode &= ~OAPPEND;
		}
		if(mode){
			werrstr("unsupported mode in create");
			goto out;
		}
		fd = open(path, umode, perm);
	}
out:
	if(fd >= 0){
		if(lock){
			/* unused by ed; in windows this requires messing with file handles */
			sysfatal("create(%s, OLOCK)", path);
		}
		if(cexec) {
			/* unused by ed -- no idea how to do it! */
			sysfatal("p9create: fcntl(fd, F_SETFL, FD_CLOEXEC);");
		}
		if(rclose)
			remove(path);
	}
	return fd;
}
