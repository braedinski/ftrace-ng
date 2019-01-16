/*
 * i386.h
 *
 * @braedinski (Braeden Lynden),
 * @AdamKinnell (Adam Kinnell),
 * @elfmaster (Ryan O'Neill) is the creator of 'ftrace'.
 *
*/

#ifndef _I386_H_
#define _I386_H_

#include "ftrace-ng.h"

#define I386_CALL	0xE8
#define I386_RETN	0xC3
#define I386_INT3	0xCC

bool i386_trace(struct process_s *);
bool i386_set_breakpoint(struct process_s *, struct breakpoint_s *);
bool i386_unset_breakpoint(struct process_s *, struct breakpoint_s *);

#endif // _I386_H_