/**************************************************************************************$
 * httpmessage
 ***************************************************************************************
 * Copyright © 2022 by Renaud Guillard (dev@nore.fr)
 * Distributed under the terms of the MIT License, see LICENSE
 ***************************************************************************************
 */

#include "httpmessage/httpmessage.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

typedef struct __httpmessage_test
{
	const char *name;
	int (*test)(int, const char **);
} httpmessage_test;

void print_line(FILE *stream, const char *text, size_t length)
{
	fprintf(stream, "[%03d] ", (int)length);
	
	while (length)
	{
		if (*text == '\r')
		{
			fprintf(stream, "\\r");
		}
		else if (*text == '\n')
		{
			fprintf(stream, "\\n");
		}
		else
		{
			fprintf(stream, "%c", *text);
		}
		
		--length;
		++text;
	}
	
	fprintf(stream, "\n");
}


int run_tests(const httpmessage_test *tests, size_t count,
              int argc, const char **argv)
{
	size_t t;
	int a;
	int exitCode = EXIT_SUCCESS;
	
	for (t = 0; t < count; ++t)
	{
		const httpmessage_test *T = &tests[t];
		
		if (argc <= 1)
		{
			goto run_tests_run_test;
		}
		
		for (a = 1; a < argc; ++a)
		{
			if (strcmp(argv[a], T->name) == 0)
			{
				goto run_tests_run_test;
			}
		}
		
		continue;
		
run_tests_run_test:
		fprintf(stdout, "## %s ################################\n", T->name);
		exitCode += (*T->test)(argc, argv);
	}
	
	return exitCode;
}
