/*
 * process.c
 *
 * @braedinski (Braeden Lynden),
 * @AdamKinnell (Adam Kinnell),
 * @elfmaster (Ryan O'Neill) is the creator of 'ftrace'.
 *
*/

#include "ftrace-ng.h"

bool process_run(struct process_s *process, struct arch_funcs_s *arch_funcs);
bool process_get_address_space(struct process_s *process);

/*
 * process_exec()
 * The function loads the binary at 'path' and then fork's and execve's it.
*/
bool process_exec(char *path, char **argv)
{
	struct arch_funcs_s arch_funcs;
	memset(&arch_funcs, 0, sizeof(struct arch_funcs_s));

	struct process_s process;
	memset(&process, 0, sizeof(struct process_s));
	process.path = path;
	process.attached = false;

	char *name = strrchr(path, '/');
	process.name = (name ? name + 1 : path);
	
	// Load symbol information
	if (elf_open_object(
		process.path,
		&process.elf.object,
		ELF_LOAD_F_STRICT,
		&process.elf.error) == false)
	{
		// If we can't parse the ELF object, we're screwed!
		fprintf(stderr,
			"The ELF header for '%s' could not be parsed.\n",
			process.path
		);
		return false;
	}

	switch (process.elf.object.arch) {
		case i386: {
			arch_funcs.run = &i386_run;
			break;
		}
		case x64: {
			arch_funcs.run = &x64_run;
			break;
		}
		default: {
			puts("Unsupported ABI");
		}
	}

	process.pid = fork();
	switch (process.pid) {
		case -1: {
			// ERROR: Unable to fork
			perror("fork");
			exit(1);
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

			// Get segment information
			if (!process_get_address_space(&process)) {
				fprintf(stderr,
					"Could not parse /proc/%d/maps, exiting...\n",
					process.pid
				);
				exit(1);
			}

			// Trace child until exit
			while (process_run(&process, &arch_funcs));
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

	return true;
}

/*
 * process_run
 *
*/
bool process_run(struct process_s *process, struct arch_funcs_s *arch_funcs)
{
	// To be replaced with PTRACE_CONT eventually, and breakpoints.
	ptrace(PTRACE_SINGLESTEP, process->pid, NULL, NULL);

	int status;
	wait(&status);
	if (WIFEXITED(status))
	{
		return false;
	}

	// The current state of all x86 registers.
	ptrace(PTRACE_GETREGS, process->pid, NULL, &process->registers);

	// We only care about the PC inside of our .text section.
	if (process->registers.rip < process->as.start ||
		process->registers.rip > process->as.end)
	{
		return true;
	}

	return arch_funcs->run(process);
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