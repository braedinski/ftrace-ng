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

bool i386_run(struct process_s *process);

#endif // _I386_H_