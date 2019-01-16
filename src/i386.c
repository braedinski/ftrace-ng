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
 * i386_get_function_arguments()
 *
*/
int i386_get_function_arguments(struct process_s *process)
{
	uint32_t instruction;

	// I'm just testing stuff don't yell at me...
	for (int displ = 4; displ != (4 * 2); displ +=4 )
	{
		instruction = ptrace(
				PTRACE_PEEKTEXT,
				process->pid,
				process->registers.rsp + displ,
				NULL
		);

		// It could do without a fixed-length buffer!
		char buffer[256];
		memset(&buffer, 0, sizeof(buffer));

		if (instruction >= process->as.start && instruction <= process->as.end) {
			int idx = 0;
			int str_displ = 0;

			bool done = false;
			while (!done) {
				long data = ptrace(
						PTRACE_PEEKTEXT,
						process->pid,
						instruction + str_displ,
						NULL
				);

				long mask = 0xff;
				for (int i = 0; i < 8; ++i) {
					uint8_t ch = (data & mask) >> (i * 8);

					if (ch == 0x00) {
						done = true;
						break;
					}

					// It'll do...
					if (ch >= 0x20 && ch <= 0x7e) {
						buffer[idx++] = ch;
					} else {
						done = true;
					}

					mask <<= 8;
				}

				str_displ += 8;
			}

			if (strlen(buffer)) {
				printf("%p @ \"%s\"\n", instruction, buffer);
			}
		}
	}

	return instruction;
}


/*
 * i386_trace
*/
bool i386_trace(struct process_s *process)
{
	struct elf_symbol symbol;

	struct breakpoint_s *bp = breakpoint_search(
		process->breakpoints,
		process->registers.rip - 1
	);

	if (!bp) {
		return true;
	}

	switch (bp->type) {
		case BT_CALL: {
			struct callret_s callret;

			// We grab the return address from %esp/%rsp
			uint32_t return_address = ptrace(
				PTRACE_PEEKTEXT,
				process->pid,
				process->registers.rsp,
				NULL
			);

			// A breakpoint is set on the return address with type BT_RETN.
			struct breakpoint_s breakpoint = {
				.address = return_address,
				.type = BT_RETN
			};

			if (i386_set_breakpoint(process, &breakpoint)) {
				breakpoint_push_back(&process->breakpoints, &breakpoint);
			}

			int argument = i386_get_function_arguments(process);

			// Some tabs to help visualise the callstack
			for (int i = 0; i < process->stack.depth; ++i) {
				putchar('\t');
			}
			
			if (elf_symbol_by_value(
				&process->elf.object,
				process->registers.rip,
				&symbol) == true)
			{
				printf("%s+%s %s(%d)%s\n",
					KGRN,
					KNRM,
					symbol.name,
					argument,
					KNRM
				);

				callret.call_address = symbol.value;
			}
			else
			{
				printf("%s+%s %p(%d)%s\n",
					KGRN,
					KNRM,
					(void *)process->registers.rip,
					argument,
					KNRM
				);

				callret.call_address = process->registers.rip;
			}
				
			callret.retn_address = return_address;
			stack_push(&process->stack, &callret);

			break;
		}
		case BT_RETN: {
			struct callret_s *callret = stack_pop(&process->stack);
			if (callret) {
				for (int i = 0; i < process->stack.depth; ++i) {
					putchar('\t');
				}

				if (elf_symbol_by_value(
					&process->elf.object,
					callret->call_address,
					&symbol) == true)
				{
					// The binary is not stripped, print the symbol names.
					printf("%s-%s %s() = %s%lld%s\n",
						KYEL,
						KNRM,
						symbol.name,
						KMAG,
						process->registers.rax,
						KNRM
					);
				}
				else
				{
					// The binary is stripped, print the addresses.
					printf("%s-%s %p() = %s%lld%s\n",
						KYEL,
						KNRM,
						(void *)callret->call_address,
						KMAG,
						process->registers.rax,
						KNRM
					);
				}
			}

			break;
		}
	}

	return true;
}
