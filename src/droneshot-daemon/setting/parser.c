#include "parser.h"

#include <ctype.h>
#include <errno.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

static char * trim(char *s)
{
	char *h = s;
	char *t = s + strlen(s) - 1;

	// remove leading whitespace.
	while (h[0] && isspace(h[0])) {
		h++;
	}

	// remove trailing whitespace.
	while (t > h && isspace(t[0])) {
		t[0] = 0;
		t--;
	}

	return h;
}

bool setting_parse(const char *file, setting_parser_t parser, void *ctx)
{
	FILE *fp;
	bool res;
	size_t ln;
	char line[256];

	// open file.
	fp = fopen(file, "r");
	if (!fp) {
		fprintf(stderr, "Failed to open %s: %s.\n", file, strerror(errno));
		return false;
	}

	// parse content.
	res = true;

	for (ln = 1; fgets(line, sizeof(line), fp); ln++) {
		const char *s = trim(line);
		char name[256], value[256];

		// skip if it is empty line or comment line.
		if (!s[0] || s[0] == '#') {
			continue;
		}

		// parse line.
		switch (sscanf(s, "%[^= \t] = %[^\n]", name, value)) {
		case 1:
			value[0] = 0;
			break;
		case 2:
			trim(value); // value might contains trailing whitespace.
			break;
		default:
			fprintf(stderr, "%s:%zu: Format of the line is not valid.\n", file, ln);
			res = false;
			goto done;
		}

		// parse value.
		if (!parser(file, ln, name, value, ctx)) {
			res = false;
			break;
		}
	}

	done:
	if (ferror(fp)) {
		fprintf(stderr, "Error while reading %s: %s.\n", file, strerror(errno));
		res = false;
	}

	// clean up.
	if (fclose(fp) == EOF) {
		fprintf(stderr, "Failed to close %s: %s.\n", file, strerror(errno));
	}

	return res;
}

bool setting_parse_pin(const char *file, size_t ln, const char *name, uint8_t *res)
{
	// we use hard-coded value for portability.
	// if we use symbolic value, it will only compilable on RPI.
	if (!strcmp(name, "RPI_V2_GPIO_P1_03")) {
		res[0] = 2;
	} else if (!strcmp(name, "RPI_V2_GPIO_P1_05")) {
		res[0] = 3;
	} else if (!strcmp(name, "RPI_V2_GPIO_P1_07")) {
		res[0] = 4;
	} else if (!strcmp(name, "RPI_V2_GPIO_P1_08")) {
		res[0] = 14;
	} else if (!strcmp(name, "RPI_V2_GPIO_P1_10")) {
		res[0] = 15;
	} else if (!strcmp(name, "RPI_V2_GPIO_P1_11")) {
		res[0] = 17;
	} else if (!strcmp(name, "RPI_V2_GPIO_P1_12")) {
		res[0] = 18;
	} else if (!strcmp(name, "RPI_V2_GPIO_P1_13")) {
		res[0] = 27;
	} else if (!strcmp(name, "RPI_V2_GPIO_P1_15")) {
		res[0] = 22;
	} else if (!strcmp(name, "RPI_V2_GPIO_P1_16")) {
		res[0] = 23;
	} else if (!strcmp(name, "RPI_V2_GPIO_P1_18")) {
		res[0] = 24;
	} else if (!strcmp(name, "RPI_V2_GPIO_P1_19")) {
		res[0] = 10;
	} else if (!strcmp(name, "RPI_V2_GPIO_P1_21")) {
		res[0] = 9;
	} else if (!strcmp(name, "RPI_V2_GPIO_P1_22")) {
		res[0] = 25;
	} else if (!strcmp(name, "RPI_V2_GPIO_P1_23")) {
		res[0] = 11;
	} else if (!strcmp(name, "RPI_V2_GPIO_P1_24")) {
		res[0] = 8;
	} else if (!strcmp(name, "RPI_V2_GPIO_P1_26")) {
		res[0] = 7;
	} else if (!strcmp(name, "RPI_V2_GPIO_P1_29")) {
		res[0] = 5;
	} else if (!strcmp(name, "RPI_V2_GPIO_P1_31")) {
		res[0] = 6;
	} else if (!strcmp(name, "RPI_V2_GPIO_P1_32")) {
		res[0] = 12;
	} else if (!strcmp(name, "RPI_V2_GPIO_P1_33")) {
		res[0] = 13;
	} else if (!strcmp(name, "RPI_V2_GPIO_P1_35")) {
		res[0] = 19;
	} else if (!strcmp(name, "RPI_V2_GPIO_P1_36")) {
		res[0] = 16;
	} else if (!strcmp(name, "RPI_V2_GPIO_P1_37")) {
		res[0] = 26;
	} else if (!strcmp(name, "RPI_V2_GPIO_P1_38")) {
		res[0] = 20;
	} else if (!strcmp(name, "RPI_V2_GPIO_P1_40")) {
		res[0] = 21;
	} else if (!strcmp(name, "RPI_V2_GPIO_P5_03")) {
		res[0] = 28;
	} else if (!strcmp(name, "RPI_V2_GPIO_P5_04")) {
		res[0] = 29;
	} else if (!strcmp(name, "RPI_V2_GPIO_P5_05")) {
		res[0] = 30;
	} else if (!strcmp(name, "RPI_V2_GPIO_P5_06")) {
		res[0] = 31;
	} else {
		fprintf(stderr, "%s:%zu: '%s' is not a valid pin name.\n", file, ln, name);
		return false;
	}

	return true;
}
