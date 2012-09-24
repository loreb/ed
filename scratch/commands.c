/* Here's a small program that triggers the bug (20120923, forgot to commit) */
#include <setjmp.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#ifdef __MINGW32__
typedef struct {
	jmp_buf buf;	/* FIXME */
	long savemask;
} sigjmp_buf[1];

typedef long p9jmp_buf[sizeof(sigjmp_buf)/sizeof(long)];
extern int sigsetjmp(sigjmp_buf, int);
extern void siglongjmp(sigjmp_buf, int);

void siglongjmp(sigjmp_buf env, int val)
{
	longjmp(env->buf, val);
}
int sigsetjmp(sigjmp_buf env, int savemask)
{
	return setjmp(env->buf);
}
void
p9longjmp(p9jmp_buf buf, int val)
{
	siglongjmp((void*)buf, val);
}
#define sleep(s) _sleep(1000*s)

extern	void	p9longjmp(p9jmp_buf, int);
extern	void	p9notejmp(void*, p9jmp_buf, int);
/* extern	int	p9setjmp(p9jmp_buf); */
#define p9setjmp(b)	sigsetjmp((void*)(b), 1)
/*
 * <stdlib.h>
extern	long	strtol(char*, char**, int);
extern	ulong	strtoul(char*, char**, int);
extern	vlong	strtoll(char*, char**, int);
extern	uvlong	strtoull(char*, char**, int);
 */
extern	void	sysfatal(char*, ...);
extern	void	p9syslog(int, char*, char*, ...);
extern	long	p9time(long*);
/* extern	int	tolower(int); <ctype.h> */
/* extern	int	toupper(int); <ctype.h> */
extern	void	needstack(int);
extern	char*	readcons(char*, char*, int);

extern	void	(*_pin)(void);
extern	void	(*_unpin)(void);

#ifndef NOPLAN9DEFINES
#define atexit		p9atexit
#define atexitdont	p9atexitdont
#define atoi		p9atoi
#define atol		p9atol
#define atoll		p9atoll
#define encrypt		p9encrypt
#define decrypt		p9decrypt
#define getenv		p9getenv
#define	getwd		p9getwd
#define	longjmp		p9longjmp
#undef  setjmp
#define setjmp		p9setjmp
#define putenv		p9putenv
#define notejmp		p9notejmp
#define jmp_buf		p9jmp_buf
#define time		p9time
#define pow10		p9pow10
#define strtod		fmtstrtod
#define charstod	fmtcharstod
#define syslog		p9syslog
#endif

#endif/* mingw */

#define t() v((char *)0)
#define v(wtf) (fprintf(stderr, "%u", __LINE__), wtf?fprintf(stderr, ": %s", wtf):0, fputc('\n', stderr), fflush(stderr))
void commands(void);
jmp_buf savej;
int main()
{
	t();
	setjmp(savej);
	v("savej");
	commands();
	v("shouldn't happen!");
	return 0;
}

static void error(const char *s)
{
	t();
	/*reset*/
	longjmp(savej, 1);
}

int *address(void)
{
	int i;
	if(printf("") == 123) {
		t();
		return &i;
	}
	t();
	error("address()");
	return 0;
}


void commands()
{
	int *a;
	sleep(1);
	t();
	a = address();
	printf("a=%p\n", a);
}
