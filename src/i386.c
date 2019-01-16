/*
 * ftrace-ng.c
 *
 * @braedinski (Braeden Lynden),
 * @AdamKinnell (Adam Kinnell),
 * @elfmaster (Ryan O'Neill) is the creator of 'ftrace'.
 *
*/

#include "ftrace-ng.h"

/*
 * i386_set_breakpoint
*/
bool i386_set_breakpoint(struct process_s *process, long address)
{
	long instruction = ptrace(PTRACE_PEEKTEXT, process->pid, address, NULL);
	long bp_instruction = (instruction & ~0xff) | I386_INT3; // 0xCC

	struct breakpoint_s breakpoint = {
		.address = address,
		.previous_instruction = instruction
	};

	breakpoint_push_back(&process->breakpoints, &breakpoint);

	ptrace(PTRACE_POKETEXT, process->pid, address, bp_instruction);

	return true;
}


/*
 * i386_unset_breakpoint
*/
bool i386_unset_breakpoint(struct process_s *process, long address)
{
	// We're always 1 byte forward after the INT3 instruction executes.
	address = process->registers.rip - 1;

	struct breakpoint_s *breakpoint = breakpoint_search(
		process->breakpoints,
		address
	);

	if (breakpoint) {
		ptrace(PTRACE_POKETEXT,
				process->pid,
				address,
				breakpoint->previous_instruction);

		process->registers.rip = address;
		ptrace(PTRACE_SETREGS, process->pid, 0, &process->registers);

		return true;
	}

	return false;
}


/*
 * i386_get_instructions
*/
bool i386_get_instructions(
	struct process_s *process,
	uint8_t *opcode)
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
 * i386_trace
*/
bool i386_trace(struct process_s *process)
{
	uint8_t opcode[8];
	i386_get_instructions(process, opcode);

	// If we can't locate the symbol, GTFO
	struct elf_symbol symbol;
	if (elf_symbol_by_value(
		&process->elf.object,
		process->registers.rip,
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

	// For tracing CALL and RET instructions
	switch (opcode[0]) {
		case 0x89: {
			break;
		}
		case I386_RETN: {
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

	return true;
}
