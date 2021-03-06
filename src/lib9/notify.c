/*
 * Signal handling for Plan 9 programs. 
 * We stubbornly use the strings from Plan 9 instead 
 * of the enumerated Unix constants.  
 * There are some weird translations.  In particular,
 * a "kill" note is the same as SIGTERM in Unix.
 * There is no equivalent note to Unix's SIGKILL, since
 * it's not a deliverable signal anyway.
 *
 * We do not handle SIGABRT or SIGSEGV, mainly because
 * the thread library queues its notes for later, and we want
 * to dump core with the state at time of delivery.
 *
 * We have to add some extra entry points to provide the
 * ability to tweak which signals are deliverable and which
 * are acted upon.  Notifydisable and notifyenable play with
 * the process signal mask.  Notifyignore enables the signal
 * but will not call notifyf when it comes in.  This is occasionally
 * useful.
 */

#include <u.h>
#include <signal.h>
#define NOPLAN9DEFINES
#include <libc.h>

extern char *_p9sigstr(int, char*);
extern int _p9strsig(char*);

typedef struct Sig Sig;
struct Sig
{
	int sig;			/* signal number */
	int flags;
};

enum
{
	Restart = 1<<0,
	Ignore = 1<<1,
	NoNotify = 1<<2,
};

/*
 * Windows signals:
 * SIGABRT
 * SIGFPE
 * SIGILL
 * SIGINT
 * SIGSEGV
 * SIGTERM
 */
static Sig sigs[] = {
#ifdef SIGHUP
	SIGHUP,		0,
#endif
	SIGINT,		0,
#ifdef SIGQUIT
	SIGQUIT,		0,
#endif
	SIGILL,		0,
#ifdef SIGTRAP
	SIGTRAP,		0,
#endif
/*	SIGABRT, 		0, 	*/
#ifdef SIGEMT
	SIGEMT,		0,
#endif
	SIGFPE,		0,
#ifdef SIGBUS
	SIGBUS,		0,
#endif
/*	SIGSEGV, 		0, 	*/
#ifdef SIGCHLD
	SIGCHLD,		Restart|Ignore,
#endif
#ifdef SIGSYS
	SIGSYS,		0,
#endif
#ifdef SIGPIPE
	SIGPIPE,		Ignore,
#endif
#ifdef SIGALRM
	SIGALRM,		0,
#endif
	SIGTERM,		0,
#ifdef SIGTSTP
	SIGTSTP,		Restart|Ignore|NoNotify,
#endif
/*	SIGTTIN,		Restart|Ignore, */
/*	SIGTTOU,		Restart|Ignore, */
#ifdef SIGXCPU
	SIGXCPU,		0,
#endif
#ifdef SIGXFSZ
	SIGXFSZ,		0,
#endif
#ifdef SIGVTALRM
	SIGVTALRM,	0,
#endif
#ifdef SIGUSR1
	SIGUSR1,		0,
#endif
#ifdef SIGUSR2
	SIGUSR2,		0,
#endif
#ifdef SIGWINCH
	SIGWINCH,	Restart|Ignore|NoNotify,
#endif
#ifdef SIGINFO
	SIGINFO,		Restart|Ignore|NoNotify,
#endif
};

static Sig*
findsig(int s)
{
	int i;

	for(i=0; i<nelem(sigs); i++)
		if(sigs[i].sig == s)
			return &sigs[i];
	return nil;
}

/*
 * The thread library initializes _notejmpbuf to its own
 * routine which provides a per-pthread jump buffer.
 * If we're not using the thread library, we assume we are
 * single-threaded.
 */
typedef struct Jmp Jmp;
struct Jmp
{
	p9jmp_buf b;
};

static Jmp onejmp;

static Jmp*
getonejmp(void)
{
	return &onejmp;
}

Jmp *(*_notejmpbuf)(void) = getonejmp;
static void noteinit(void);

/*
 * Actual signal handler. 
 */

static void (*notifyf)(void*, char*);	/* Plan 9 handler */

static void
signotify(int sig)
{
	char tmp[64];
	Jmp *j;
	Sig *s;

	j = (*_notejmpbuf)();
	switch(p9setjmp(j->b)){
	case 0:
		if(notifyf)
			(*notifyf)(nil, _p9sigstr(sig, tmp));
		/* fall through */
	case 1:	/* noted(NDFLT) */
		if(0)print("DEFAULT %d\n", sig);
		s = findsig(sig);
		if(s && (s->flags&Ignore))
			return;
		signal(sig, SIG_DFL);
		raise(sig);
		_exit(1);
	case 2:	/* noted(NCONT) */
		if(0)print("HANDLED %d\n", sig);
		/*
		 * Windows does "signal(sig, SIG_DFL)" before invoking
		 * the signal handler, like SysV.
		 * MSDN says it's a "feature".
		 */
		signal(sig, signotify);
		return;
	}
}

static void
signonotify(int sig)
{
	USED(sig);
}

int
noted(int v)
{
	p9longjmp((*_notejmpbuf)()->b, v==NCONT ? 2 : 1);
	abort();
	return 0;
}

int
notify(void (*f)(void*, char*))
{
	static int init;

	notifyf = f;
	if(!init){
		init = 1;
		noteinit();
	}
	return 0;
}

/*
 * Nonsense about enabling and disabling signals.
 */
typedef void Sighandler(int);
static Sighandler*
handler(int s)
{
	Sighandler *curr;

	curr = signal(s, SIG_DFL);
	if(SIG_ERR == signal(s, curr))
		sysfatal("handler: %r");
	return curr;
}

static int
notesetenable(int sig, int enabled)
{
#ifdef SIG_BLOCK
	sigset_t mask, omask;

	if(sig == 0)
		return -1;

	sigemptyset(&mask);
	sigaddset(&mask, sig);
	sigprocmask(enabled ? SIG_UNBLOCK : SIG_BLOCK, &mask, &omask);
	return !sigismember(&omask, sig);
#else
	sysfatal("notesetenable unimplemented!");
	return -1;
#endif	
}

int
noteenable(char *msg)
{
	return notesetenable(_p9strsig(msg), 1);
}

int
notedisable(char *msg)
{
	return notesetenable(_p9strsig(msg), 0);
}

static int
notifyseton(int s, int on)
{
	Sighandler *sigh, *oldh;
	Sig *sig;

	sig = findsig(s);
	if(sig == nil)
		return -1;
	sigh = on ? signotify : signonotify;
	if(sig->flags&Restart)
		/* none on win32: this is an assertion */
		sysfatal("sa.sa_flags |= SA_RESTART");

	/*
	 * We can't allow signals within signals because there's
	 * only one jump buffer.
	 */
	/* sigfillset(&sa.sa_mask); */
	/* FIXME equivalent in windows? */

	/*
	 * Install handler.
	 */
	oldh = signal(sig->sig, sigh);
	return oldh == signotify;
}

int
notifyon(char *msg)
{
	return notifyseton(_p9strsig(msg), 1);
}

int
notifyoff(char *msg)
{
	return notifyseton(_p9strsig(msg), 0);
}

/*
 * Initialization follows sigs table.
 */
static void
noteinit(void)
{
	int i;
	Sig *sig;

	for(i=0; i<nelem(sigs); i++){
		sig = &sigs[i];
		/*
		 * If someone has already installed a handler,
		 * It's probably some ld preload nonsense,
		 * like pct (a SIGVTALRM-based profiler).
		 * Or maybe someone has already called notifyon/notifyoff.
		 * Leave it alone.
		 */
		if(handler(sig->sig) != SIG_DFL)
			continue;
		notifyseton(sig->sig, !(sig->flags&NoNotify));
	}
}

