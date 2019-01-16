/*
 * simple.c
 * A simple program for testing CALL/RET.
*/

int function_2(int x)
{
	return 32;
}

int function_1(int x)
{
	int y = function_2(24);

	return 16;
}

int main()
{
	function_1(8);
	function_1(4);
	
	return 0;
}