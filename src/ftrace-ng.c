/*
 * ftrace-ng.c
 *
 * @braedinski (Braeden Lynden),
 * @AdamKinnell (Adam Kinnell),
 * @elfmaster (Ryan O'Neill) is the creator of 'ftrace'.
 *
*/

#include "ftrace-ng.h"

#include <getopt.h>

void options();

/*
static struct option long_options[] =
{
    {"load", 1, NULL, 'l'},
    {"attach", 0, NULL, 'a'},
    {NULL, 0, NULL, 0}
};
*/

/*
 * main()
*/
int main(int argc, char **argv, char **envp)
{
	printf("ftrace-ng (%d.%d)\n", VERSION_MAJOR, VERSION_MINOR);

	if (argc < 2) {
		fprintf(stderr, "Usage: %s <filename> <args>... \n", argv[0]);
		options();
		return EXIT_FAILURE;
	}

	/*
	int ch;
	while ((ch = getopt_long(argc, argv, "l:a:", long_options, NULL)) != -1) {
		switch (ch) {
			case 'l': {
				printf("Loading '%s'\n", optarg);
				//field.title = optarg;
				break;
			}
			case 'a': {
				printf("Attaching to %s\n", optarg);
				//field.artist = optarg;
				break;
			}
		}
	}
	*/

	char *path = argv[1];
	if (!process_exec(path, &argv[1])) {
		fprintf(stderr, "The file '%s' could not be loaded, exiting...\n", argv[1]);
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

void options()
{
	puts("-");
}