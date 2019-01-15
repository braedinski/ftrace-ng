#
# ftrace-ng Makefile
#

all: $(wildcard *.c)
	gcc -pg \
	-std=c99 \
	-Wall \
	-D_GNU_SOURCE \
	-I/opt/elfmaster/include \
	-L/opt/elfmaster/lib \
	$(wildcard *.c) \
	-o ftrace-ng \
	-lelfmaster
	
clean:
	rm -f ftrace-ng callstack