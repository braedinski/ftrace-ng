/*
 * breakpoint.c
 *
 * @braedinski (Braeden Lynden),
 * @AdamKinnell (Adam Kinnell),
 * @elfmaster (Ryan O'Neill) is the creator of 'ftrace'.
 *
*/

#include "breakpoint.h"

/*
 * breakpoint_push_back()
*/
void breakpoint_push_back(
	struct breakpoint_s **head,
	struct breakpoint_s *breakpoint)
{
	if (!breakpoint->address) {
		return;
	}

	// To start a new list
	struct breakpoint_s *current = *head;
	if (current == NULL) {
		current = calloc(1, sizeof(struct breakpoint_s));
		if (current) {
			memcpy(current, breakpoint, sizeof(struct breakpoint_s));
			current->next = NULL;
			*head = current;	
		} else {
			perror("calloc");
		}

		return;
	}

	// To append to the end of an established list
	while (current) {
		if (!current->next) {
			current->next = calloc(1, sizeof(struct breakpoint_s));
			if (current->next) {
				memcpy(current->next, breakpoint, sizeof(struct breakpoint_s));
				current->next->next = NULL;
			} else {
				perror("calloc");
			}

			break;
		}

		current = current->next;
	}
}


/*
 * breakpoint_print()
*/
void breakpoint_print(struct breakpoint_s *current)
{
	while (current) {
		// print
		current = current->next;
	}
}


/*
 * breakpoint_search()
*/
struct breakpoint_s *breakpoint_search(
	struct breakpoint_s *head,
	long address)
{
	struct breakpoint_s *current = head;
	while (current) {
		if (current->address == address) {
			return current;
		}

		if (!current->next) {
			break;
		}

		current = current->next;
	}

	return NULL;
}

/*
 * breakpoint_destroy_list()
*/
bool breakpoint_destroy_list(struct breakpoint_s *head)
{
	// Who cares about freeing memory
	return true;
}
