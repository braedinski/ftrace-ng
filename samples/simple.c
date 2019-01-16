/*
 * simple.c
 * A simple program for testing CALL/RET.
*/

#include <string.h>

char *string = "Oh wow, I can read strings now? I'm so intelligent.";

int function_2(int x)
{
	return 32;
}

int function_1(int x)
{
	int y = function_2(24);

	return 16;
}

void function_print(char *ptr)
{
	return;
}

int main()
{
	char buf[256];
	strcpy(buf, "Hello, World");

	function_1(8);
	function_1(4);
	function_2(16);
	function_1(48);
	function_print(string);
	
	return 0;
}