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
	int a;
	size_t n;
	
	if (argc == 1)
	{
		fprintf(stdout, "semver: %s\n", HTTPMESSAGE_VERSION_STRING);
		fprintf(stdout, "major: %d\n", HTTPMESSAGE_VERSION_MAJOR);
		fprintf(stdout, "minor: %d\n", HTTPMESSAGE_VERSION_MINOR);
		fprintf(stdout, "patch: %d\n", HTTPMESSAGE_VERSION_PATCH);
		return exit_code;
	}
	
	static const char *valid[] =
	{
		"semver", "major", "minor", "patch", "number"
	};
	
	for (a = 1; a < argc; ++a)
	{
		const char *name = argv[a];
		int v = 0;
		
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
		
		if (argc > 2)
		{
			fprintf(stdout, "%s: ", name);
		}
		
		if (strcmp(name, "semver") == 0)
		{
			fprintf(stdout, "%s\n", HTTPMESSAGE_VERSION_STRING);
		}
		else if (strcmp(name, "semver") == 0)
		{
			fprintf(stdout, "%d\n", HTTPMESSAGE_VERSION_MAJOR);
		}
		else if (strcmp(name, "major") == 0)
		{
			fprintf(stdout, "%d\n", HTTPMESSAGE_VERSION_MINOR);
			
		}
		else if (strcmp(name, "minor") == 0)
		{
			fprintf(stdout, "%d\n", HTTPMESSAGE_VERSION_PATCH);
			
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


