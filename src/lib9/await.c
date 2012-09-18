/*
 * This file is only here to convert string<-->signal;
 * the wait*() functions  are unused -- the original ed
 * forks and waits immediately, I'll just spawn()
 */
#define NOPLAN9DEFINES
#include <u.h>
#include <libc.h>

#include <signal.h>
#include <sys/types.h>
#include <sys/time.h>

#ifndef WCOREDUMP	/* not on Mac OS X Tiger */
#define WCOREDUMP(status) 0
#endif

static struct {
	int sig;
	char *str;
} tab[] = {
#ifdef SIGHUP
	SIGHUP,		"hangup",
#endif
#ifdef SIGINT
	SIGINT,		"interrupt",
#endif
#ifdef SIGQUIT
	SIGQUIT,		"quit",
#endif
#ifdef SIGILL
	SIGILL,		"sys: illegal instruction",
#endif
#ifdef SIGTRAP
	SIGTRAP,		"sys: breakpoint",
#endif
#ifdef SIGABRT
	SIGABRT,		"sys: abort",
#endif
#ifdef SIGEMT
	SIGEMT,		"sys: emulate instruction executed",
#endif
#ifdef SIGFPE
	SIGFPE,		"sys: fp: trap",
#endif
#ifdef SIGKILL
	SIGKILL,		"sys: kill",
#endif
#ifdef SIGBUS
	SIGBUS,		"sys: bus error",
#endif
#ifdef SIGSEGV
	SIGSEGV,		"sys: segmentation violation",
#endif
#ifdef SIGALRM
	SIGALRM,		"alarm",
#endif
#ifdef SIGTERM
	SIGTERM,		"kill",
#endif
#ifdef SIGURG
	SIGURG,		"sys: urgent condition on socket",
#endif
#ifdef SIGSTOP
	SIGSTOP,		"sys: stop",
#endif
#ifdef SIGTSTP
	SIGTSTP,		"sys: tstp",
#endif
#ifdef SIGCONT
	SIGCONT,		"sys: cont",
#endif
#ifdef SIGCHLD
	SIGCHLD,		"sys: child",
#endif
#ifdef SIGTTIN
	SIGTTIN,		"sys: ttin",
#endif
#ifdef SIGTTOU
	SIGTTOU,		"sys: ttou",
#endif
#ifdef SIGIO	/* not on Mac OS X Tiger */
	SIGIO,		"sys: i/o possible on fd",
#endif
#ifdef SIGXCPU
	SIGXCPU,		"sys: cpu time limit exceeded",
#endif
#ifdef SIGXFSZ
	SIGXFSZ,		"sys: file size limit exceeded",
#endif
#ifdef SIGVTALRM
	SIGVTALRM,	"sys: virtual time alarm",
#endif
#ifdef SIGPROF
	SIGPROF,		"sys: profiling timer alarm",
#endif
#ifdef SIGWINCH	/* not on Mac OS X Tiger */
	SIGWINCH,	"sys: window size change",
#endif
#ifdef SIGINFO
	SIGINFO,		"sys: status request",
#endif
#ifdef SIGUSR1
	SIGUSR1,		"sys: usr1",
#endif
#ifdef SIGUSR2
	SIGUSR2,		"sys: usr2",
#endif
#ifdef SIGPIPE
	SIGPIPE,		"sys: write on closed pipe",
#endif
};
	
char*
_p9sigstr(int sig, char *tmp)
{
	int i;

	for(i=0; i<nelem(tab); i++)
		if(tab[i].sig == sig)
			return tab[i].str;
	if(tmp == nil)
		return nil;
	sprint(tmp, "sys: signal %d", sig);
	return tmp;
}

int
_p9strsig(char *s)
{
	int i;

	for(i=0; i<nelem(tab); i++)
		if(strcmp(s, tab[i].str) == 0)
			return tab[i].sig;
	return 0;
}

static int
_await(int pid4, char *str, int n, int opt)
{
#ifdef WTERMSIG
	int pid, status, cd;
	struct rusage ru;
	char buf[128], tmp[64];
	ulong u, s;

	for(;;){
		/* On Linux, pid==-1 means anyone; on SunOS, it's pid==0. */
		if(pid4 == -1)
			pid = wait3(&status, opt, &ru);
		else
			pid = wait4(pid4, &status, opt, &ru);
		if(pid <= 0)
			return -1;
		u = ru.ru_utime.tv_sec*1000+((ru.ru_utime.tv_usec+500)/1000);
		s = ru.ru_stime.tv_sec*1000+((ru.ru_stime.tv_usec+500)/1000);
		if(WIFEXITED(status)){
			status = WEXITSTATUS(status);
			if(status)
				snprint(buf, sizeof buf, "%d %lud %lud %lud %d", pid, u, s, u+s, status);
			else
				snprint(buf, sizeof buf, "%d %lud %lud %lud ''", pid, u, s, u+s, status);
			strecpy(str, str+n, buf);
			return strlen(str);
		}
		if(WIFSIGNALED(status)){
			cd = WCOREDUMP(status);
			snprint(buf, sizeof buf, "%d %lud %lud %lud 'signal: %s%s'", pid, u, s, u+s, _p9sigstr(WTERMSIG(status), tmp), cd ? " (core dumped)" : "");
			strecpy(str, str+n, buf);
			return strlen(str);
		}
	}
#else
	sysfatal("where on earth does ed call _await()?");
	return -1;
#endif
}

int
await(char *str, int n)
{
	return _await(-1, str, n, 0);
}

int
awaitnohang(char *str, int n)
{
#ifdef WNOHANG
	return _await(-1, str, n, WNOHANG);
#else
	return -1;
#endif
}

int
awaitfor(int pid, char *str, int n)
{
	return _await(pid, str, n, 0);
}

