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
bool i386_set_breakpoint(
	struct process_s *process,
	struct breakpoint_s *breakpoint)
{
	if (!breakpoint) {
		return false;
	}

	// We need to record the instruction that was at this address.
	breakpoint->previous_instruction = ptrace(
		PTRACE_PEEKTEXT,
		process->pid,
		breakpoint->address,
		NULL
	);

	// This POKETEXT call sets the first byte of the instruction to 'INT 3'.
	ptrace(
		PTRACE_POKETEXT,
		process->pid,
		breakpoint->address,
		(breakpoint->previous_instruction & ~0xff) | I386_INT3
	);

	return true;
}


/*
 * i386_unset_breakpoint
*/
bool i386_unset_breakpoint(
	struct process_s *process,
	struct breakpoint_s *breakpoint)
{
	if (!breakpoint) {
		return false;
	}

	// We're always 1 byte forward after the INT3 instruction executes.
	long address = process->registers.rip - 1;

	ptrace(PTRACE_POKETEXT,
			process->pid,
			address,
			breakpoint->previous_instruction
	);

	process->registers.rip = address;
	ptrace(PTRACE_SETREGS, process->pid, 0, &process->registers);

	return true;
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

	// The return address is located on le stack
	unsigned long return_address = ptrace(
		PTRACE_PEEKTEXT,
		process->pid,
		process->registers.rsp,
		NULL
	);

	return_address &= 0xffffffff;

	for (int i = 0; i < process->stack.depth; ++i) {
		putchar('\t');
	}

	printf("%s+%s %s()%s @ %p from %p\n",
		KGRN,
		KNRM,
		symbol.name,
		KNRM,
		(void *)symbol.value,
		(void *)return_address
	);

	// For tracing CALL and RET instructions
	switch (opcode[0]) {
		case I386_CALL: {
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

	struct callret_s callret = {
		.address = process->registers.rip,
		.return_address = return_address
	};

	stack_push(&process->stack, &callret);

	// For setting a breakpoint on the return from the current function.
	/*
	long instruction = ptrace(PTRACE_PEEKTEXT, process->pid, return_address, NULL);
	long bp_instruction = (instruction & ~0xff) | I386_INT3; // 0xCC
	ptrace(PTRACE_POKETEXT, process->pid, return_address, bp_instruction);
	*/

	return true;
}
