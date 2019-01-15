/*
 * process.c
 *
 * @braedinski (Braeden Lynden),
 * @AdamKinnell (Adam Kinnell),
 * @elfmaster (Ryan O'Neill) is the creator of 'ftrace'.
 *
*/

#include "include/ftrace-ng.h"

bool process_run(struct process_s *process);
bool process_get_address_space(struct process_s *process);

/*
 * process_get_instructions
*/
bool process_get_instructions(
	struct process_s *process,
	uint8_t *opcode,
	size_t length)
{
	long inst = ptrace(
		PTRACE_PEEKTEXT,
		process->pid,
		process->registers.rip,
		NULL
	);

	opcode[0] = (inst & 0xff);
	opcode[1] = (inst & 0xff00) >> 8;
	opcode[2] = (inst & 0xff0000) >> 16;
	opcode[3] = (inst & 0xff000000) >> 24;

	return true;
}

/*
 * process_exec()
 * The function loads the binary at 'path' and then fork's and execve's it.
 *
*/
bool process_exec(char *path, char **argv)
{
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
			puts("i386");
			break;
		}
		case x64: {
			puts("x64");
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
			while (process_run(&process));
		}
	}

	// Cleanup
	elf_close_object(&process.elf.object);
	
	return true;
}

/*
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
 *
*/
bool process_run(struct process_s *process)
{
	uint8_t opcode[8];

	int status;

	ptrace(PTRACE_SINGLESTEP, process->pid, NULL, NULL);

	wait(&status);
	if (WIFEXITED(status))
	{
		return false;
	}

	// The current state of all x86 registers
	ptrace(PTRACE_GETREGS, process->pid, NULL, &process->registers);

	// by dereferencing the PC we can obtain the instructions
	process_get_instructions(process, opcode, 8);

	// We only care about the PC inside of our .text section
	if (process->registers.rip >= process->as.start && 
		process->registers.rip <= process->as.end) {

		// If we can't locate the symbol, GTFO
		struct elf_symbol symbol;
		if (elf_symbol_by_value(
			&process->elf.object,
			process->registers.rip,
			&symbol) == false)
		{
			/* 
			* The semantics of this return statement are RIDICULOUS.
			* I think we should probably use an enum of error codes or something
			*/
			return true;
		}

		// For tracing CALL and RET instructions
		switch (opcode[0]) {
			case X86_32_CALL: {
				// It's 'hard-coded' at the moment, need to fix get_opcode_at_ip()
				unsigned long branch_address = 
				(opcode[1] + (opcode[2] << 8) + (0xff << 16) + (0xff << 24));

				struct callret_s call = {
					.address = process->registers.rip + branch_address + 0x5,
					.return_address = process->registers.rip + 0x5
				};

				// printf("%p -> %p\n", call.address, call.return_address);

				if (elf_symbol_by_value(
					&process->elf.object,
					call.address,
					&symbol) == false)
				{
					return true;
				}

				for (int i = 0; i < process->stack.depth; ++i) {
					putchar('\t');
				}

				printf("%s+ %s()%s\n",
					KGRN,
					symbol.name,
					KNRM
				);

				stack_push(&process->stack, &call);

				break;
			}
			case X86_32_RET: {
				struct callret_s *ret = stack_pop(&process->stack);
				if (!ret) {
					return true;
				}

				if (elf_symbol_by_value(
					&process->elf.object,
					ret->address,
					&symbol) == false)
				{
					return true;
				}

				for (int i = 0; i < process->stack.depth; ++i) {
					putchar('\t');
				}

				printf("%s- %s()%s -> (%s%lld%s)\n",
					KYEL,
					symbol.name,
					KNRM,
					KMAG,
					process->registers.rax,
					KNRM
				);

				break;
			}
		}
	}

	return true;
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