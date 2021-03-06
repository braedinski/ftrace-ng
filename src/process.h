/*
 * process.h
 *
 * @braedinski (Braeden Lynden),
 * @AdamKinnell (Adam Kinnell),
 * @elfmaster (Ryan O'Neill) is the creator of 'ftrace'.
 *
*/

#ifndef _PROCESS_H_
#define _PROCESS_H_

#include "ftrace-ng.h"

/*
 * address_space_s
 * To store fields parsed from /proc/<pid>/maps
*/
struct address_space_s
{
	unsigned long   start;
	unsigned long   end;
	unsigned int    size;

	const char      *path; // requires a call to free()
};


/*
 * elf_s
 * For libelfmaster to store its bits and pieces.
*/
struct elf_s
{
	elfobj_t		object;
	elf_error_t		error;
};


/*
 * arch_funcs_s
 * A set of architecture-dependent functions (i386/x64), meant to be
 * extensible.
*/
struct process_s;
struct breakpoint_s;
struct arch_funcs_s
{
	bool (*trace)(struct process_s *);
	bool (*set_breakpoint)(struct process_s *, struct breakpoint_s *);
	bool (*unset_breakpoint)(struct process_s *, struct breakpoint_s *);
};


/*
 * process_s
 *
*/
struct process_s
{
	pid_t                   pid;

	struct address_space_s  as;

	// We store a handle to the ELF object for libelfmaster
	struct elf_s            elf;

	// CPU registers updated each instruction
	struct user_regs_struct registers;

	// The callstack for this process
	struct stack_s          stack;

	struct arch_funcs_s		arch_funcs;

	// (at the moment) a linked-list of all breakpoints we've set.
	struct breakpoint_s		*breakpoints;

	// The absolute pathname to the executable
	char                    *path;

	// If we loaded the program, this is the filename argument passed from the CLI
	char                    *name;

	// Whether or not we attached or execve()'d
	bool                    attached;

	// Was the binary stripped?
	bool					stripped;
};


// These are for loading and attaching to executables, called from main()
bool process_exec(char *path, char **argv);
bool process_attach(const pid_t pid);

#endif // _PROCESS_H_