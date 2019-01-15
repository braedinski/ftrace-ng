ftrace-ng TODO
================================================================================
1. The ftrace-ng.c file should (ideally) only contain the main() function,
	but it's okay for now.

2. I don't know whether the get_process_address_space() function is needed at 
	all.

3. The get_opcodes_at_instruction() function doesn't return the required # of
	opcodes for calculating the branching address of a CALL instruction.
	At the moment it's just: 
		0xE8 <1> <2> <3>, it should be
		0xE8 <1> <2> <3> <4>.

	The branch (target) address of a (near) CALL is calculated on x86 using:

	400504:	e8 dd ff ff ff       	call   4004e6 <function_2>

	target = (0x400504 + 0x5 + 0xffffffdd) & 0xffffffff
		   = 0x4004e6

4. The next feature required is getting the return-value from EAX after
	RET/0xC3.

5. Eventually the program should be able to decipher function arguments,
	for example, strcpy(buffer, "Hello, World");

6. For messages sent to stdout/in/err file descriptors, they should be wrapped
	in ftrace's output using "[std(out|in|err)]
	<message sent to std(out|err) of traced program>".

7. If you think of other features, let me know >____< .
