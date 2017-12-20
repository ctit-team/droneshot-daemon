#include "argument.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h>

static struct arguments current;

static void print_usage(const char *prog, FILE *output)
{
	fprintf(output, "Usage: %s [-h] [-v CATEGORY]\n", prog);
}

static bool parse_verbose(const char *value, enum logging_category *res)
{
	const char * const delim = ",";
	char *buf, *name;
	enum logging_category cats;

	// check if it enable all categories.
	if (!strcmp(value, "all")) {
		res[0] = ~(enum logging_category)0;
		return true;
	}

	// prepare workspace.
	buf = strdup(value);
	if (!buf) {
		fprintf(stderr, "Insufficient memory to parse verbose mode.\n");
		return false;
	}

	// splitting category.
	name = strtok(buf, delim);
	cats = logging_none; // value might be empty string.

	if (name) {
		do {
			if (!strcmp(name, "hardware")) {
				cats |= logging_hardware;
			} else {
				fprintf(stderr, "Unknown logging category '%s'.\n", name);
				free(buf);
				return false;
			}
		} while ((name = strtok(NULL, delim)) != NULL);
	}

	free(buf);

	res[0] = cats;
	return true;
}

enum argument_init_result argument_init(int argc, char *argv[])
{
	int ch;

	while ((ch = getopt(argc, argv, ":hv:")) != -1) {
		switch (ch) {
		case 'h':
			print_usage(argv[0], stdout);
			return argument_init_shown_help;
		case 'v':
			if (!parse_verbose(optarg, &current.logging_enabled)) {
				return argument_init_error;
			}
			break;
		case ':':
		case '?':
			print_usage(argv[0], stderr);
			return argument_init_error;
		}
	}

	return argument_init_ok;
}

void argument_term(void)
{
}

const struct arguments * argument_current(void)
{
	return &current;
}
