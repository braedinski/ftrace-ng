#include <stdio.h>

int function_1(int a, int b, int c, int d, int e, int f, int g, int h, char *i, int j, float k)
{
	return 1;
}

int main()
{
	/* fld loads the floating-point value into the ST(0) register,
	 * the lea instruction is to set the location for where we want
	 * to store the floating-point value on the stack */

	/*
	 8048413:	d9 05 e4 84 04 08    	fld    DWORD PTR ds:0x80484e4	; We can grab the symbol for this.
	 8048419:	8d 64 24 fc          	lea    esp,[esp-0x4]
	 804841d:	d9 1c 24             	fstp   DWORD PTR [esp]
	 8048420:	6a 08                	push   0x8
	 8048422:	68 d4 84 04 08       	push   0x80484d4				; and this;
	 8048427:	6a 07                	push   0x7
	 8048429:	6a 06                	push   0x6
	 804842b:	6a 05                	push   0x5
	 804842d:	6a 04                	push   0x4
	 804842f:	6a 03                	push   0x3
	 8048431:	6a 02                	push   0x2
	 8048433:	6a 01                	push   0x1
	 8048435:	6a 00                	push   0x0
	 8048437:	e8 ca ff ff ff       	call   8048406 <function_1>
 	*/

 	/* We need to figure out where the arguments begin */

	function_1(0, 1, 2, 3, 4, 5, 6, 0xffff, "Hello, World!", 8, 1.5f);
	return 0;
}