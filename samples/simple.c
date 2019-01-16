/*
 * simple.c
 * A simple program for testing CALL/RET.
*/

#include <string.h>

char *string = "Oh wow, I can read strings now? I'm so intelligent.";

int function_2(int x);

void function_print(char *ptr)
{
	return;
}

int function_1(int x)
{
	int y = function_2(24);

	return 16;
}

int function_2(int x)
{
	return 32;
}

int function_3(int x, int y, int z)
{
	return 0;
}

int main()
{
	function_1(8);
	function_2(16);
	function_3(1, 2, 3);
	function_print(string);
	
	return 0;
}