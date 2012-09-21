/*
 * Why not opentemp? because it uses mkstemp, unavailable in windows.
 * This function makes it easier to deal with windows _not_ having
 * a fixed location for temp files
 * -- which is a good thing, it provides each user with its $TEMP,
 * kinda like plan9, except it doesn't have a unique name...
 */
#include <u.h>
#include <libc.h>
#define WIN32_LEAN_AND_MEAN	/* avoid plan9/win32 conflicts */
#include <windows.h>

/*
 * ed.c has its own mktemp
 * This function considers filename an _output_ parameter:
 * on success it is filled with the temporary filename.
 */
int mktempfd(char filename[MAX_PATH], int mode, int perm)
{
	char temp_path[MAX_PATH];
	char temp_file[MAX_PATH];
	unsigned long dw;
	int fd, again;

	filename[0] = '\0';	/* ignoring errors should be hard */

	/* One of these GetTemp* actually returns uint - quickly, which one? */
	dw = GetTempPath(MAX_PATH, temp_path);
	if (dw > MAX_PATH || (dw == 0)) {
		fprint(2, "GetTempPath: error %lu (ret=%lu)\n",
		    (unsigned long)GetLastError(), dw);
		/* try using $PWD -- "." => ".\ed12345" */
		if(!getcwd(temp_path, MAX_PATH))
			return -1;
	}
	for(again = 42; again > 0; again --) {
		/* Generates a temporary file name. */
		dw = GetTempFileName(temp_path, /* directory for tmp files */
		"edXXXXXXXXXX",        /* temp file name prefix */
		1,                    /* let ME create it! */
		temp_file);          /* buffer for name */
		if(dw == 0) {
			werrstr("GetTempFileName: error %lu",
			    (unsigned long)GetLastError());
			continue;
		}
		fd = create(temp_file, OEXCL | mode, perm);
		if(fd >= 0) {
			strcpy(filename, temp_file);
			return fd;
		}
	}
	return -1;
}
