#define _GNU_SOURCE
#include <memory.h>
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <ucontext.h>
#include <dlfcn.h>
#include <execinfo.h>

#if defined(REG_RIP)
# define SIGSEGV_STACK_IA64
# define REGFORMAT "%016lx"
#elif defined(REG_EIP)
# define SIGSEGV_STACK_X86
# define REGFORMAT "%08x"
#else
# define SIGSEGV_STACK_GENERIC
# define REGFORMAT "%x"
#endif

static void signal_segv(int signum, siginfo_t* info, void*ptr) 
{
	//static const char *si_codes[3] = {"", "SEGV_MAPERR", "SEGV_ACCERR"};			

#if defined(SIGSEGV_STACK_X86) || defined(SIGSEGV_STACK_IA64)
	int f = 0;
	Dl_info dlinfo;
	void **bp = 0;
	void *ip = 0;
#else
	void *bt[20];
	char **strings;
	size_t sz;
#endif

#if defined(SIGSEGV_STACK_X86) || defined(SIGSEGV_STACK_IA64)
#if defined(SIGSEGV_STACK_IA64)
	ucontext_t *ucontext = (ucontext_t*)ptr;
	ip = (void*)ucontext->uc_mcontext.gregs[REG_RIP];
	bp = (void**)ucontext->uc_mcontext.gregs[REG_RBP];
#elif defined(SIGSEGV_STACK_X86)
	ucontext_t *ucontext = (ucontext_t*)ptr;
	ip = (void*)ucontext->uc_mcontext.gregs[REG_EIP];
	bp = (void**)ucontext->uc_mcontext.gregs[REG_EBP];
#endif

	fprintf(stderr, "Stack trace:\n");
	while(bp && ip) 
	{
	    if(!dladdr(ip, &dlinfo))
		    break;
        const char *symname = dlinfo.dli_sname;
        fprintf(stderr, "% 2d: %p %s+%u (%s)\n",
		        ++f,
		        ip,
		        symname,
		        (unsigned)((char*)ip - (char*)dlinfo.dli_saddr),
		        dlinfo.dli_fname);

        if(dlinfo.dli_sname && !strcmp(dlinfo.dli_sname, "main"))
            break;

        ip = bp[1];
        bp = (void**)bp[0];
    }

#else
	fprintf(stderr, "Stack trace (non-dedicated):\n");
	sz = backtrace(bt, 20);
	strings = backtrace_symbols(bt, sz);
		
	int i;
	for(i = 0; i < sz; ++i)
	    fprintf(stderr, "%s\n", strings[i]);
#endif
	fprintf(stderr, "End of stack trace\n");
	return;
}


// compile : gcc -o funstack -rdynamic -ldl dump.c
// signal: 
// SIGABRT
// SIGSEGV
//
// you also can call the function "raise(signum)" to handle the signal 
int setup_sigsegv(int signum) 
{
    struct sigaction action;
    memset(&action, 0, sizeof(action));
    action.sa_sigaction = signal_segv;
    action.sa_flags = SA_SIGINFO;
    if(sigaction(signum, &action, NULL) < 0) 
	{
    	perror("sigaction");
	    return 0;
	}

    return 1;
}
				
