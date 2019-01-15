#
# ftrace-ng Makefile
# It's really bad at the moment...
#

all: ftrace-ng.c callstack.c
	gcc -g -D_GNU_SOURCE -I/opt/elfmaster/include -L/opt/elfmaster/lib ftrace-ng.c callstack.c -o ftrace-ng -lelfmaster

clean:
	rm -f ftrace-ng callstack