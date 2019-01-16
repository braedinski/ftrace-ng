ftrace-ng TODO
================================================================================
* I don't know whether the get_process_address_space() function is needed at all.
* The next feature required is getting the return-value from EAX after RET/0xC3.
* Eventually the program should be able to decipher function arguments, for example, strcpy(buffer, "Hello, World");
* For messages sent to stdout/in/err file descriptors, they should be wrapped in ftrace's output using "std(out|in|err) <message sent to std(out|err) of traced program>".
* If you think of other features, let me know >____< .
