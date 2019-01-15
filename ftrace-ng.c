/*
 * ftrace-ng.c
 *
 * @braedinski (Braeden Lynden),
 * @AdamKinnell (Adam Kinnell),
 * @elfmaster (Ryan O'Neill) is the creator of 'ftrace'.
 *
*/

#include "include/ftrace-ng.h"

void options();

/*
 * main()
 *
*/
int main(int argc, char **argv, char **envp)
{
	puts("ftrace-ng");

	if (argc != 2) {
		fprintf(stderr, "Usage: %s <filename>\n", argv[0]);
        options();
		return EXIT_FAILURE;
	}

    char *path = argv[1];
    if (!process_exec(path)) {
        fprintf(stderr, "The file '%s' could not be loaded, exiting...\n", argv[1]);
        return EXIT_FAILURE;
    }

	return EXIT_SUCCESS;
}

void options()
{
    printf("");
}