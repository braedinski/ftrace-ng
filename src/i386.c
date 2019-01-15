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
 * i386_run
*/
bool i386_run(struct process_s *process)
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

	return true;
}
