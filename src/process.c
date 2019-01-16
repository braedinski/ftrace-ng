/*
 * process.c
 *
 * @braedinski (Braeden Lynden),
 * @AdamKinnell (Adam Kinnell),
 * @elfmaster (Ryan O'Neill) is the creator of 'ftrace'.
 *
*/

#include "ftrace-ng.h"

bool process_trace(struct process_s *);
bool process_get_address_space(struct process_s *);

/*
 * process_cleanup
 * For deallocating resources used by 'struct process_s'.
*/
void process_cleanup(struct process_s *process)
{

}


/*
 * process_set_arch_funcs
 * Sets up required function-pointers for handling different architectures.
*/
bool process_set_arch_funcs(
	struct process_s *process)
{
	assert(process != NULL);

	switch (process->elf.object.arch) {
		case i386: {
			process->arch_funcs.trace = &i386_trace;
			process->arch_funcs.set_breakpoint = &i386_set_breakpoint;
			process->arch_funcs.unset_breakpoint = &i386_unset_breakpoint;
			return true;
		}
		case x64: {
			process->arch_funcs.trace = &x64_trace;
			return true;
		}
		default: {
		}
	}

	return false;
}


/*
 * process_set_breakpoints()
*/
int process_set_breakpoints(struct process_s *process)
{
	int count = 0;

	assert(process != NULL);
	assert(process->arch_funcs.set_breakpoint != NULL);

	elf_symtab_iterator_t iterator;
	struct elf_symbol symbol;

	elf_symtab_iterator_init(&process->elf.object, &iterator);
	while (elf_symtab_iterator_next(&iterator, &symbol) == ELF_ITER_OK) {
    	if (symbol.type == 2 /* I dunno why there isn't a macro */) {
    		struct breakpoint_s breakpoint = {
				.address = symbol.value,
				.previous_instruction = 0,
				.type = BT_CALL
			};
			
    		if (process->arch_funcs.set_breakpoint(process, &breakpoint)) {
    			breakpoint_push_back(&process->breakpoints, &breakpoint);
    			count++;
    		}
		}
	}

	if (!count)
	{
		// If we got here, the binary must be stRIPped.
		process->stripped = true;

		elf_eh_frame_iterator_t iter;
		struct elf_eh_frame fde;

		printf("We couldn't locate any symbols, trying to rebuild using GNU_EH_FRAME\n");

	    elf_eh_frame_iterator_init(&process->elf.object, &iter);
	    while (elf_eh_frame_iterator_next(&iter, &fde) == ELF_ITER_OK) {
	    	struct breakpoint_s breakpoint = {
				.address = fde.pc_begin,
				.previous_instruction = 0,
				.type = BT_CALL
			};
			
    		if (process->arch_funcs.set_breakpoint(process, &breakpoint)) {
    			breakpoint_push_back(&process->breakpoints, &breakpoint);
    			count++;
    		}
		}
	}
	else
	{
		// We had some breakpoints set, so it's not stripped?!
		process->stripped = false;
	}

	return count;
}


/*
 * process_exec()
 * The function loads the binary at 'path' and then fork's and execve's it.
*/
bool process_exec(char *path, char **argv)
{
	struct process_s process;
	memset(&process, 0, sizeof(struct process_s));
	process.path = path;
	process.attached = false;

	char *name = strrchr(path, '/');
	process.name = (name ? name + 1 : path);
	
	// For parsing the ELF object at 'path' using libelfmaster.
	if (elf_open_object(
		process.path,
		&process.elf.object,
		ELF_LOAD_F_SMART|ELF_LOAD_F_FORENSICS,
		&process.elf.error) == false)
	{
		fprintf(stderr,
			"The ELF header for '%s' could not be parsed.\n",
			process.path
		);
		return false;
	}

	// This function depends on the ELF object being parsed correctly.
	if (!process_set_arch_funcs(&process))
	{
		fprintf(stderr, "The architecture is not supported.\n");
		return false;
	}

	process.pid = fork();
	switch (process.pid) {
		case -1: {
			// ERROR: Unable to fork
			perror("fork");
			break;
		}
		case 0: {
			// CHILD: Start the process to be traced.
			if (ptrace(PTRACE_TRACEME, 0, NULL, NULL) == -1) {
				perror("ptrace");
				exit(1);
			}
			ptrace(PTRACE_SETOPTIONS, 0, 0, PTRACE_O_TRACEEXIT);

			// Start child process
			if (execve(process.path, argv, NULL) == -1) {
				perror("execve");
				exit(1);
			}
		}
		default: {
			// PARENT: Begin tracing the child process.
			int status;
			wait(&status);

			// We set breakpoints on all symbols
			if (process_set_breakpoints(&process) <= 0) {
				fprintf(stderr,
					"We couldn't find any functions to trace, exiting...\n"
				);

				break;
			}

			breakpoint_print(process.breakpoints);

			// Get segment information
			if (!process_get_address_space(&process)) {
				fprintf(stderr,
					"Could not parse /proc/%d/maps, exiting...\n",
					process.pid
				);
				break;
			}

			// Trace child until exit
			while (process_trace(&process) == true);
		}
	}

	// Cleanup
	elf_close_object(&process.elf.object);
	
	return true;
}


/*
 * process_attach
 *
*/
bool process_attach(const pid_t pid)
{
	struct process_s process;
	memset(&process, 0, sizeof(struct process_s));

	process.attached = true;
	process.pid = pid;

	if (ptrace(PTRACE_ATTACH, process.pid, NULL, NULL) == -1) {
		perror("ptrace");
		return false;
	}

	while (process_trace(&process) == true);

	// Cleanup
	process_cleanup(&process);

	return true;
}


/*
 * process_trace
 * 
*/
bool process_trace(struct process_s *process)
{
	bool rv = false;

	// To be replaced with PTRACE_CONT eventually, and breakpoints.
	ptrace(PTRACE_CONT, process->pid, NULL, NULL);

	int status;
	wait(&status);

	// The child process exited, so we exit.
	if (WIFEXITED(status)) {
		return false;
	}

	// We hit a breakpoint, so let the arch-func handle it.
	if (WIFSTOPPED(status)) {
		// The current state of all CPU registers.
		ptrace(PTRACE_GETREGS, process->pid, NULL, &process->registers);

		rv = process->arch_funcs.trace(process);

		struct breakpoint_s *breakpoint = breakpoint_search(
			process->breakpoints,
			process->registers.rip - 1
		);

		process->arch_funcs.unset_breakpoint(process, breakpoint);

		// Thanks Adam!
		ptrace(PTRACE_SINGLESTEP, process->pid, NULL, NULL);
		wait(&status);

		process->arch_funcs.set_breakpoint(process, breakpoint);
	}

	return rv;
}


/*
 * get_process_address_space()
 * Parses /proc/<pid>/maps to obtain the virtual addresses for this process.
*/
bool process_get_address_space(struct process_s *process)
{
	FILE *fd;

	char path[32];
	char line[256];

	// The current line # we're parsing
	unsigned int lc = 0;

	// The path is formatted as /proc/<pid>/maps
	snprintf(path, sizeof(path), "/proc/%d/maps", process->pid);
	
	if ((fd = fopen(path, "rb")) == NULL) {
		return false;
	}

	// Read a line at a time, 256 bytes max
	while (fgets(line, sizeof(line), fd)) {
		char *offset;

		// To extract the first two virtual addresses, separated by '-'
		unsigned long start = strtoul(line, &offset, 16);
		unsigned long end = strtoul(offset + 1, &offset, 16);

		// The permissions field is always going to be 4 bytes, i.e. 'rwxp'
		char *permissions = strndup(
			strchr(offset, ' ') + 1,
			PARSER_PERMISSIONS_LENGTH
		);

		offset += PARSER_PERMISSIONS_LENGTH;

		/* 
		* If '/' exists in the line, it's an absolute path,
		* otherwise it's a [stack] or [heap] object.
		*/
		char *object_path = strchr(offset, '/');
		object_path = (object_path == NULL) ? strchr(offset, '[') : object_path;

		if (object_path && strstr(object_path, process->name) && lc == 0)
		{
			process->as.start = start;
			process->as.end = end;
			process->path = strdup(object_path);
		}

		// strndup() -> free()
		free(permissions);

		lc++;
	}

	return true;
}