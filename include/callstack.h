/*
 * callstack.h
  *
 * @braedinski (Braeden Lynden),
 * @AdamKinnell (Adam Kinnell),
 * @elfmaster (Ryan O'Neill) is the creator of 'ftrace'.
 *
*/

#ifndef _CALLSTACK_H_
#define _CALLSTACK_H_

#include <string.h>
#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>

/*
 * callret_s
*/
struct callret_s
{
    unsigned long address;
	unsigned long return_address;

	struct callret_s *next;
};


/*
 * stack_s
*/
struct stack_s
{
	int depth;

	struct callret_s *top;
};

/*
 *
*/
bool stack_push(struct stack_s *stack, struct callret_s *callret);

struct callret_s *stack_pop(struct stack_s *stack);
struct callret_s *stack_top(struct stack_s *stack);

#endif // _CALLSTACK_H_