#define _GNU_SOURCE
#include <wordexp.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <signal.h>
#include <errno.h>
#include <fcntl.h>
#include <sched.h>
#include "pthread_impl.h"

static void reap(pid_t pid)
{
	int status;
	while (waitpid(pid, &status, 0) < 0 && errno == EINTR);
}

static char *getword(FILE *f)
{
	char *s = 0;
	return getdelim(&s, (size_t [1]){0}, 0, f) < 0 ? 0 : s;
}

struct forked_args {
	const char *s;
	char *redir;
	int *p;
};

static int do_wordexp_forked(void *args_) {
	struct forked_args *args = args_;
	if (args->p[1] == 1) fcntl(1, F_SETFD, 0);
	else dup2(args->p[1], 1);
	execl("/bin/sh", "sh", "-c",
		"eval \"printf %s\\\\\\\\0 x $1 $2\"",
		"sh", args->s, args->redir, (char *)0);
	_exit(1);
}

static int do_wordexp(const char *s, wordexp_t *we, int flags)
{
	size_t i, l;
	int sq=0, dq=0;
	size_t np=0;
	char *w, **tmp;
	char *redir = (flags & WRDE_SHOWERR) ? "" : "2>/dev/null";
	int err = 0;
	FILE *f;
	size_t wc = 0;
	char **wv = 0;
	int p[2];
	pid_t pid;
	sigset_t set;

	if (flags & WRDE_REUSE) wordfree(we);

	if (flags & WRDE_NOCMD) for (i=0; s[i]; i++) switch (s[i]) {
	case '\\':
		if (!sq && !s[++i]) return WRDE_SYNTAX;
		break;
	case '\'':
		if (!dq) sq^=1;
		break;
	case '"':
		if (!sq) dq^=1;
		break;
	case '(':
		if (np) {
			np++;
			break;
		}
	case ')':
		if (np) {
			np--;
			break;
		}
	case '\n':
	case '|':
	case '&':
	case ';':
	case '<':
	case '>':
	case '{':
	case '}':
		if (!(sq|dq|np)) return WRDE_BADCHAR;
		break;
	case '$':
		if (sq) break;
		if (s[i+1]=='(' && s[i+2]=='(') {
			i += 2;
			np += 2;
			break;
		} else if (s[i+1] != '(') break;
	case '`':
		if (sq) break;
		return WRDE_CMDSUB;
	}

	if (flags & WRDE_APPEND) {
		wc = we->we_wordc;
		wv = we->we_wordv;
	}

	i = wc;
	if (flags & WRDE_DOOFFS) {
		if (we->we_offs > SIZE_MAX/sizeof(void *)/4)
			goto nospace;
		i += we->we_offs;
	} else {
		we->we_offs = 0;
	}

	if (pipe2(p, O_CLOEXEC) < 0) goto nospace;
	__block_all_sigs(&set);
	char child_stack[4096];
	struct forked_args args = {
		.s = s,
		.redir = redir,
		.p = p,
	};
  pid = clone(do_wordexp_forked, child_stack + sizeof(child_stack),
              CLONE_VM | CLONE_VFORK | SIGCHLD, &args);
  __restore_sigs(&set);
	if (pid < 0) {
		close(p[0]);
		close(p[1]);
		goto nospace;
	}
	close(p[1]);
	
	f = fdopen(p[0], "r");
	if (!f) {
		close(p[0]);
		kill(pid, SIGKILL);
		reap(pid);
		goto nospace;
	}

	l = wv ? i+1 : 0;

	free(getword(f));
	if (feof(f)) {
		fclose(f);
		reap(pid);
		return WRDE_SYNTAX;
	}

	while ((w = getword(f))) {
		if (i+1 >= l) {
			l += l/2+10;
			tmp = realloc(wv, l*sizeof(char *));
			if (!tmp) break;
			wv = tmp;
		}
		wv[i++] = w;
		wv[i] = 0;
	}
	if (!feof(f)) err = WRDE_NOSPACE;

	fclose(f);
	reap(pid);

	if (!wv) wv = calloc(i+1, sizeof *wv);

	we->we_wordv = wv;
	we->we_wordc = i;

	if (flags & WRDE_DOOFFS) {
		if (wv) for (i=we->we_offs; i; i--)
			we->we_wordv[i-1] = 0;
		we->we_wordc -= we->we_offs;
	}
	return err;

nospace:
	if (!(flags & WRDE_APPEND)) {
		we->we_wordc = 0;
		we->we_wordv = 0;
	}
	return WRDE_NOSPACE;
}

int wordexp(const char *restrict s, wordexp_t *restrict we, int flags)
{
	int r, cs;
	pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, &cs);
	r = do_wordexp(s, we, flags);
	pthread_setcancelstate(cs, 0);
	return r;
}

void wordfree(wordexp_t *we)
{
	size_t i;
	if (!we->we_wordv) return;
	for (i=0; i<we->we_wordc; i++) free(we->we_wordv[we->we_offs+i]);
	free(we->we_wordv);
	we->we_wordv = 0;
	we->we_wordc = 0;
}
