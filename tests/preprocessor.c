/**************************************************************************************$
 * httpmessage
 ***************************************************************************************
 * Copyright © 2022 by Renaud Guillard (dev@nore.fr)
 * Distributed under the terms of the MIT License, see LICENSE
 ***************************************************************************************
 */

#include "httpmessage/httpmessage.h"
#include "shared.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

int test_text_pointer_size(int argc, const char **argv);

int test_text_pointer_size(int argc, const char **argv)
{
	(void) argc;
	(void) argv;
	int exitCode = EXIT_SUCCESS;
	size_t pointer_size = sizeof(void *);
	
	if (HTTPMESSAGE_POINTER_SIZE != pointer_size)
	{
		fprintf(stderr, "Invalid value of HTTPMESSAGE_POINTER_SIZE\n"
		        "\t%-48.48s: %d\n"
		        "\t%-48.48s: %d\n"
		        ,
		        "HTTPMESSAGE_POINTER_SIZE", (int) HTTPMESSAGE_POINTER_SIZE,
		        "pointer_size", (int)pointer_size
		       );
		++exitCode;
	}
	
	return exitCode;
}

int main(int argc, const char **argv)
{
	static const httpmessage_test tests[] =
	{
		{"text_pointer_size", test_text_pointer_size }
	};
	
	return run_tests(tests, sizeof(tests) / sizeof(httpmessage_test),
	                 argc, argv);
}
