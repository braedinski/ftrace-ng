#
# ftrace-ng Makefile
#

all: $(wildcard src/*.c)
	mkdir -p bin
	gcc -pg \
	-std=c99 \
	-Wall \
	-D_GNU_SOURCE \
	-I/opt/elfmaster/include \
	-L/opt/elfmaster/lib \
	$(wildcard src/*.c) \
	-o bin/ftrace-ng \
	-lelfmaster
	cp bin/ftrace-ng ./ftrace-ng
	
clean:
	rm -f ftrace-ng