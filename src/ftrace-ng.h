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

#include "breakpoint.h"
#include "callstack.h"
#include "process.h"
#include "i386.h"
#include "x64.h"

#define PARSER_PERMISSIONS_LENGTH 4 // rwxp

#define VERSION_MAJOR 1
#define VERSION_MINOR 0

#define KNRM  "\x1B[0m"
#define KRED  "\x1B[91m"
#define KGRN  "\x1B[92m"
#define KYEL  "\x1B[93m"
#define KBLU  "\x1B[94m"
#define KMAG  "\x1B[95m"
#define KCYN  "\x1B[96m"
#define KWHT  "\x1B[37m"

#endif // _FTRACE_NG_H_ 