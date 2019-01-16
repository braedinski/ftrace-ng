/*
 * breakpoint.h
 *
 * @braedinski (Braeden Lynden),
 * @AdamKinnell (Adam Kinnell),
 * @elfmaster (Ryan O'Neill) is the creator of 'ftrace'.
 *
*/

#ifndef _BREAKPOINT_H_
#define _BREAKPOINT_H_

#include "ftrace-ng.h"

/*
 * A linked-list?! It's so inefficient, but it'll do for now.
 * *cries* ...
*/
struct breakpoint_s
{
	// char *symbol;
	long address;
	long previous_instruction;

	struct breakpoint_s *next;
};


void breakpoint_print(struct breakpoint_s *);
void breakpoint_push_back(struct breakpoint_s **, struct breakpoint_s *);
struct breakpoint_s *breakpoint_search(struct breakpoint_s *, long);

#endif // _BREAKPOINT_H_