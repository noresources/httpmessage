/**************************************************************************************$
 * httpmessage
 ***************************************************************************************
 * Copyright © 2022 by Renaud Guillard (dev@nore.fr)
 * Distributed under the terms of the MIT License, see LICENSE
 ***************************************************************************************
 */

#include "httpmessage/version.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, const char **argv)
{
	int exit_code = 0;
	int a, start;
	size_t n;
	static const char *valid[] =
	{
		"semver", "number", "major", "minor", "patch"
	};
	
	start = 1;
	
	if (argc == 1)
	{
		start = 0;
		argv = valid;
		argc = (int)(sizeof(valid) / sizeof(const char *));
	}
	
	for (a = start; a < argc; ++a)
	{
		const char *name = argv[a];
		int v = 0;
		
		if (argc == 1)
		{
			goto main_process;
		}
		
		for (n = 0; n < (sizeof(valid) / sizeof(const char *)); ++n)
		{
			if (strcmp(name, valid[n]) == 0)
			{
				v = 1;
				break;
			}
		}
		
		if (v == 0)
		{
			fprintf(stderr, "error: %s is not a valid key.\n", name);
			exit_code = a;
			break;
		}
		
main_process:

		if (argc > 2)
		{
			fprintf(stdout, "%s: ", name);
		}
		
		if (strcmp(name, "semver") == 0)
		{
			fprintf(stdout, "%s\n", HTTPMESSAGE_VERSION_STRING);
		}
		else if (strcmp(name, "major") == 0)
		{
			fprintf(stdout, "%d\n", HTTPMESSAGE_VERSION_MAJOR);
		}
		else if (strcmp(name, "minor") == 0)
		{
			fprintf(stdout, "%d\n", HTTPMESSAGE_VERSION_MINOR);
		}
		else if (strcmp(name, "patch") == 0)
		{
			fprintf(stdout, "%d\n", HTTPMESSAGE_VERSION_PATCH);
		}
		else if (strcmp(name, "number") == 0)
		{
			fprintf(stdout, "%d\n", HTTPMESSAGE_VERSION_NUMBER);
		}
		
	}
	
	return exit_code;
}


