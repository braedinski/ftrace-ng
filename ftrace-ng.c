/*
 * ftrace-ng.c
 *
 * @braedinski (Braeden Lynden), 2019
 * @elfmaster (Ryan O'Neill) is the creator of 'ftrace'.
 *
*/

#include "include/ftrace-ng.h"

/*
 * get_process_address_space()
 * Parses /proc/<pid>/maps to obtain the virtual addresses for this process.
*/
int get_process_address_space(struct process_s *process)
{
	assert(process != NULL);

	FILE *fd;

	char path[32];
	char line[256];

	/* The current line # we're parsing */
	unsigned int lc = 0;

	/* The path is formatted as /proc/<pid>/maps */
	snprintf(path, sizeof(path), "/proc/%d/maps", process->pid);
	
	if ((fd = fopen(path, "rb")) == NULL)
	{
		return -1;
	}
	
	/* Read a line at a time, 256 characters max */
	while (fgets(line, sizeof(line), fd))
	{
		/* The current offset of our parser within the line */
		char *offset;
		char *ptr = line;

		/* To extract the first two virtual addresses, separated by '-' */
		unsigned long start = strtoul(line, &offset, 16);
		unsigned long end = strtoul(offset + 1, &offset, 16);

		/* The permissions field is always going to be 4 bytes, 'rwxp' */
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

		if (object_path && strstr(object_path, process->filename) && lc == 0)
		{
			process->as.start = start;
			process->as.end = end;
			process->path = strdup(object_path);
		}

		/* strndup() -> free() */
		free(permissions);

		lc++;
	}

	return 1;
}

/*
 * get_opcodes_at_pc
*/
bool get_opcodes_at_pc(struct process_s *process, uint8_t *opcode, size_t length)
{
	long inst = ptrace(
		PTRACE_PEEKTEXT,
		process->pid,
		process->registers.rip,
		NULL);

	opcode[0] = inst & 0xff;
	opcode[1] = (inst & 0xff00) >> 8;
	opcode[2] = (inst & 0xff0000) >> 16;
	opcode[3] = (inst & 0xff000000) >> 24;

	return true;
}

/*
 * main()
 *
*/
int main(int argc, char **argv, char **envp)
{
	puts("ftrace-ng");

	if (argc != 2)
	{
		fprintf(stderr, "Usage: %s <filename>\n", argv[0]);
		return EXIT_FAILURE;
	}

	struct process_s tracee;
	memset(&tracee, 0, sizeof(tracee));

	tracee.filename = argv[1];
	tracee.attached = true;

	/* If we can't parse the ELF object, we're screwed! */
	if (elf_open_object(
		tracee.filename,
		&tracee.elf.object,
		ELF_LOAD_F_STRICT,
		&tracee.elf.error) == false)
	{
		fprintf(stderr, "libelfmaster failed to open '%s'\n", tracee.filename);
		exit(EXIT_FAILURE);
	}

	tracee.pid = fork();
	switch (tracee.pid)
	{
		case 0:
		{
			if (ptrace(PTRACE_TRACEME, 0, NULL, NULL) == -1)
			{
				perror("ptrace");
				exit(EXIT_FAILURE);
			}

			ptrace(PTRACE_SETOPTIONS, 0, 0, PTRACE_O_TRACEEXIT);

			if (execve(tracee.filename, NULL, NULL) == -1)
			{
				perror("execve");
				exit(EXIT_FAILURE);
			}

			break;
		}
		case -1:
		{
			perror("fork");
			exit(EXIT_FAILURE);
		}
		default:
		{
			int status;
			long instruction;

			uint8_t opcode[8];

			wait(&status);

			get_process_address_space(&tracee);

			while (1)
			{
				/* We choose to process one instruction at a time */
				ptrace(PTRACE_SINGLESTEP, tracee.pid, NULL, NULL);

				wait(&status);
				if (WIFEXITED(status))
				{
					break;
				}

				/* The current state of all x86 registers */
				ptrace(PTRACE_GETREGS, tracee.pid, NULL, &tracee.registers);

				/* by dereferencing the PC we can obtain the instructions */
				get_opcodes_at_pc(&tracee, opcode, 8);

				/* We only care about the PC inside of our .text section */
				if (tracee.registers.rip >= tracee.as.start && 
					tracee.registers.rip <= tracee.as.end)
				{
					/* If we can't locate the symbol, GTFO */
					struct elf_symbol symbol;
					if (elf_symbol_by_value(
							&tracee.elf.object,
							tracee.registers.rip,
							&symbol) == false)
					{
						continue;
					}

					/* For tracing CALL and RET instructions */
					switch (opcode[0])
					{
						case X86_32_CALL:
						{
							printf("%s%s%s() [%p] %s%02x%s\n",
								KBLU,
								symbol.name,
								KNRM,
								tracee.registers.rip,
								KGRN,
								opcode[0],
								KNRM
							);

                            unsigned long branch_address = 
                                (opcode[1] + (opcode[2] << 8) + (0xff << 16) + (0xff << 24));

							struct callret_s call = {
								.address = tracee.registers.rip + branch_address + 0x5,
								.return_address = tracee.registers.rip + 0x5
							};

							stack_push(&tracee.stack, &call);

							break;
						}
						case X86_32_RET:
						{
							printf("%s%s%s() [%p] -> %s%02x%s\n",
								KBLU,
								symbol.name,
								KNRM,
								tracee.registers.rip,
								KRED,
								opcode[0],
								KNRM
							);

							struct callret_s *ret = stack_pop(&tracee.stack);
                            if (ret)
                            {

                            }
                            
							break;
						}
					}
				}
			}

			elf_close_object(&tracee.elf.object);
		}
	}

	return EXIT_SUCCESS;
}