/*
 * callstack.c
 *
 * @braedinski (Braeden Lynden),
 * @AdamKinnell (Adam Kinnell),
 * @elfmaster (Ryan O'Neill) is the creator of 'ftrace'.
 *
*/

#include "include/ftrace-ng.h"

/*
 * stack_push()
*/
bool stack_push(struct stack_s *stack, struct callret_s *callret)
{
	assert(stack != NULL);

	struct callret_s *new = calloc(1, sizeof(struct callret_s));
	if (new)
	{
		memcpy(new, callret, sizeof(callret));
		new->next = stack->top;

		stack->top = new;
		stack->depth++;

		return true;
	}

	return false;
}


/*
 * stack_pop()
*/
struct callret_s *stack_pop(struct stack_s *stack)
{
	assert(stack != NULL);

	if (stack->top)
	{
			/*
			 * We need to free() this but who is responsible?!
			 * I think maybe a 2nd parameter should reference a stack variable,
			 * then we memcpy() into it.
			*/

		struct callret_s *callret = stack->top;
		if (callret)
		{
			stack->top = callret->next;
			stack->depth--;
		}

		return callret;
	}
}


/*
 * stack_top()
*/
struct callret_s *stack_top(struct stack_s *stack)
{
	return (stack && stack->top ? stack->top : NULL);
}


/*
 * main()
*/

#ifdef __DEBUG
int main(int argc, char **argv)
{
	struct stack_s stack = {
		.depth = 0
	};

	stack_pop(&stack);

	struct callret_s callret = {
		.address = 0x1
	};

	struct callret_s callret2 = {.address = 0x2, .next = 0x0};
	stack_push(&stack, &callret);
	stack_push(&stack, &callret2);


	struct callret_s *rv = stack_pop(&stack);
	printf("%p\n", rv->address);

	rv = stack_pop(&stack);
	printf("%p\n", rv->address);

	rv = stack_pop(&stack);

	return 0;
}
#endif // __DEBUG