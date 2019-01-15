/*
 * ftrace-ng.h
 *
*/

#ifndef _FTRACE_NG_H_
#define _FTRACE_NG_H_

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/user.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/ptrace.h>
#include <libelfmaster.h>

#include "callstack.h"
#include "process.h"

#define PARSER_PERMISSIONS_LENGTH 4 // rwxp

#define X86_32_CALL	0xE8
#define X86_32_RET	0xC3

#define VERSION_MAJOR 1
#define VERSION_MINOR 0

#define KNRM  "\x1B[0m"
#define KRED  "\x1B[31m"
#define KGRN  "\x1B[32m"
#define KYEL  "\x1B[33m"
#define KBLU  "\x1B[34m"
#define KMAG  "\x1B[35m"
#define KCYN  "\x1B[36m"
#define KWHT  "\x1B[37m"

#endif // _FTRACE_NG_H_ 