/*
 * ftrace-ng.h
 *
*/

#ifndef _FTRACE_NG_H_
#define _FTRACE_NG_H_

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/user.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/ptrace.h>
#include <libelfmaster.h>

/* Local includes */
#include "callstack.h"


#define PARSER_PERMISSIONS_LENGTH 4 /* 'rwxp' */


/* x86 instructions */
#define X86_32_CALL	0xE8
#define X86_32_RET	0xC3


/* I stole these, hehe? */
#define KNRM  "\x1B[0m"
#define KRED  "\x1B[31m"
#define KGRN  "\x1B[32m"
#define KYEL  "\x1B[33m"
#define KBLU  "\x1B[34m"
#define KMAG  "\x1B[35m"
#define KCYN  "\x1B[36m"
#define KWHT  "\x1B[37m"


/*
 * address_space_s
 * To store fields parsed from /proc/<pid>/maps
*/
struct address_space_s
{
	unsigned long	start;
	unsigned long	end;
	unsigned int	size;

	const char 		*path; /* requires a free() */
};


/*
 * elf_s
 *
*/
struct elf_s
{
	elfobj_t object;
	elf_error_t error;
};


/*
 * process_s
 *
*/
struct process_s
{
	/* The process identifier? */
	pid_t 					pid;

	struct address_space_s 	as;

	/* We store a handle to the ELF object for libelfmaster */
	struct elf_s 			elf;

	/* All CPU registers (rax, rbx, rdx, ..., rip) */
	struct user_regs_struct	registers;

	/* The callstack for the current process */
	struct stack_s			stack;

	char 					*path;

	/* If we loaded the process, this is the filename argument passed from the CLI */
	char 					*filename;

	/* Did we attach or did we execve()? */
	bool					attached;
};

#endif /* _FTRACE_NG_H_ */