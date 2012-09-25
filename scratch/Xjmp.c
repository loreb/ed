#include <setjmp.h>
#include <stdio.h>
#include <stdlib.h>

#ifndef __MINGW32__
#include <unistd.h>
#else
#include <windows.h>
static void sleep(unsigned n)
{
	while(n--)
		Sleep(1000);
}
#endif

typedef struct {
	jmp_buf buf;
	int savemask;
} myjmp[1];
int mysetjmp(myjmp buf, int sm)
{
	buf->savemask = sm;
	return setjmp(buf->buf);
}
void mylongjmp(myjmp buf, int val)
{
	longjmp(buf->buf, val);
}

typedef struct {
	jmp_buf buf;
	int savemask;
} sjmp_buf[1];
void ssavesigs(sjmp_buf e, int savemask)
{
	e->savemask = savemask;
}
void srestoresigs(sjmp_buf buf)
{
	if(buf->savemask)
		fprintf(stderr, "restore sigmask!\n");
}

#ifdef MY
#	define wtf "myjmp"
#	define JMP       myjmp
#	define SET(e, s) mysetjmp(e, s)
#	define GO(e, v)  mylongjmp(e, v)
#elif defined(SIG)
#	define wtf "sigsetjmp"
#	define JMP sigjmp_buf
#	define SET(e, s) sigsetjmp(e, s)
#	define GO(e, v)  siglongjmp(e, v)
#elif defined(COMMA)
#	define wtf "(sigmask,setjmp)"
#	define JMP sjmp_buf
#	define SET(e, s) (ssavesigs(e, s), setjmp(e->buf))
#	define GO(e, v)  (srestoresigs(e), longjmp(e->buf, v))
#else
#	define wtf "setjmp"
#	define JMP jmp_buf
#	define SET(e, s) ((void)s, setjmp(e))
#	define GO(e, v)  longjmp(e, v)
#endif

#ifndef NOTRACE
static void trace(const char *s, unsigned lineno)
{
	fflush(NULL);
	fprintf(stderr, "%u: %s\n", lineno, s);
	fflush(stderr);
}
#define t(what) do { trace(#what, __LINE__); what; } while(0)
#else
#define t(code) code
#endif

int *address(void);
void commands(void);
void quit(void);
JMP savej;
int main()
{
	fprintf(stderr, "using(%s);\n", wtf);
	t(SET(savej, 1));
	t(sleep(1));
	commands();
	quit();
	return 123;
}
int *address(void)
{
	static int x;

	if(fprintf(stderr, "%c", '&') == 1234)
		return &x;
	GO(savej, 1);
	return 0;
}
void commands(void)
{
	int *p;

	t(p = address());
	if(p)
		t(printf("p = %p\n", p));
	else
		t((void)p);
	return;
}
void quit(void)
{
	t(exit(0));
}
