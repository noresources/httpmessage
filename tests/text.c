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

typedef struct __compare_test
{
	const char *a;
	const char *b;
	int compared;
	int caseless_compared;
} __attribute__((packed))  compare_test;

int test_text_compare(int argc, const char **argv);
int test_quoted_string(int argc, const char **argv);

int test_text_compare(int argc, const char **argv)
{
	(void) argc;
	(void) argv;
	int exitCode = EXIT_SUCCESS;
	size_t a;
	
	static const compare_test tests[] =
	{
		{"", "", 0, 0},
		{"abc", "abc", 0, 0},
		{"abc", "abd", ('c' - 'd'), ('c' - 'd')},
		{ "abc", "Abc", ('a' - 'A'), 0 },
		{ "a", "ab", -1, -1 },
		{ "a", "abc", -2, -2 },
		{ "abc", "a", 2, 2 },
		{ "Content-Type", "CONTENT-TYPE", ('o' - 'O'), 0 }
	};
	
	for (a = 0; a < sizeof(tests) / sizeof(compare_test); ++a)
	{
		const compare_test *T = &tests[a];
		size_t sa = strlen(T->a);
		size_t sb = strlen(T->b);
		httpmessage_stringview string;
		int compared;
		int caseless_compared;
		string.text = T->a;
		string.length = sa;
		
		fprintf(stdout, "-- '%s' vs '%s' ---------------------------------\n", T->a, T->b);
		
		compared = httpmessage_stringview_compare_text(&string, T->b, sb);
		caseless_compared = httpmessage_stringview_caseless_compare_text(
		                        &string, T->b, sb);
		                        
		fprintf(stdout, "%-10.10s: %d",
		        "Compared", compared);
		        
		if (T->compared != compared)
		{
			++exitCode;
			fprintf(stdout, " <=-> expected %d\n", T->compared);
		}
		
		fprintf(stdout, "\n");
		
		fprintf(stdout, "%-10.10s: %d",
		        "Caseless", caseless_compared);
		        
		if (T->caseless_compared != caseless_compared)
		{
			++exitCode;
			fprintf(stdout, " <=-> expected %d\n", T->caseless_compared);
		}
		
		fprintf(stdout, "\n");
		
		
	}
	
	return exitCode;
}

typedef struct __quoted_string_test
{
	const char *input;
	ssize_t quoted_string_length;
	int consumed_8;
	int consumed_24;
	const char *quoted_string;
} quoted_string_test;

int test_text_quoted_string(int argc, const char **argv)
{
	int exitCode = EXIT_SUCCESS;
	int a;
	
	static const quoted_string_test tests [] =
	{
		{"\"\"", 0, 2, 2, ""},
		{ NULL, HTTPMESSAGE_ERROR_INVALID_ARGUMENT, HTTPMESSAGE_ERROR_INVALID_ARGUMENT, HTTPMESSAGE_ERROR_INVALID_ARGUMENT, "" },
		{ "\"Hello\"", 5, 7, 7, "Hello" },
		{ "\"Hello", HTTPMESSAGE_ERROR_INCOMPLETE, HTTPMESSAGE_ERROR_INCOMPLETE, HTTPMESSAGE_ERROR_INCOMPLETE, "" },
		{ "token", HTTPMESSAGE_ERROR_SYNTAX, HTTPMESSAGE_ERROR_SYNTAX, HTTPMESSAGE_ERROR_SYNTAX, "" },
		{ "\"Hello world\" ", 11, HTTPMESSAGE_ERROR_OVERFLOW, 13, "Hello world" },
		{ "\"12345678\"", 8, HTTPMESSAGE_ERROR_OVERFLOW, 10, "12345678" },
		{ "\"1234567\"", 7, 9, 9, "12345678" },
		{ "\"The € symbol is not US-ASCII character\"", HTTPMESSAGE_ERROR_SYNTAX, HTTPMESSAGE_ERROR_SYNTAX, HTTPMESSAGE_ERROR_SYNTAX, "" },
		{ "\"Quoted \\\"pair\\\"\"", 13, HTTPMESSAGE_ERROR_OVERFLOW, 17, "Quoted \"pair\"" },
		{ "\"Esc\\\fCTL\"", 7, 10, 10, "Esc\fCTL" }
	};
	
	if (argc > 1)
	{
		char quoted_string[512];
		
		for (a = 1; a < argc; ++a)
		{
			const char *text = argv[a];
			int consumed;
			ssize_t quoted_string_length;
			size_t length = strlen(text);
			
			quoted_string_length = httpmessage_quoted_string_get_length(text, length);
			
			if (quoted_string_length < 0)
			{
				fprintf(stderr, "%s: length error %d\n",
				        text, (int) quoted_string_length);
				return EXIT_FAILURE;
			}
			
			consumed = httpmessage_quoted_string_consume(
			               quoted_string, sizeof(quoted_string),
			               text, length);
			               
			if (consumed < 0)
			{
				fprintf(stderr, "%s: consume error %d\n",
				        text, consumed);
				return EXIT_FAILURE;
			}
			
			fprintf(stdout, "<%s> (%d)\n\t -> %d <%s> (%d)\n",
			        text, (int)length,
			        consumed, quoted_string, (int)quoted_string_length);
		}
		
		return EXIT_SUCCESS;
	}
	
	
	for (a = 0; a < sizeof(tests) / sizeof(quoted_string_test); ++a)
	{
		const quoted_string_test *T = &tests[a];
		const char *text = T->input;
		size_t length = (text ? strlen(text) : 0);
		char output[128];
		ssize_t quoted_string_length;
		int consumed_8;
		int consumed_24;
		
		memset(output, 0xFF, sizeof(output));
		
		fprintf(stdout, "-- '%s' ---------------------------------\n",
		        text);
		        
		quoted_string_length = httpmessage_quoted_string_get_length(text, length);
		consumed_8 = httpmessage_quoted_string_consume(
		                 output, (size_t)8,
		                 text, length);
		                 
		consumed_24 = httpmessage_quoted_string_consume(
		                  output, (size_t)24,
		                  text, length);
		                  
		fprintf(stdout,
		        "%-10.10s: %d/%d | %d/%d\n"
		        "%-10.10s: %d\n"
		        ,
		        "Consumed ", consumed_8, 8, consumed_24, 24,
		        "Length", (int)(quoted_string_length));
		        
		if (quoted_string_length >= 0)
		{
			fprintf(stdout, "%-10.10s: %s\n", "String", output);
		}
		
		if (T->quoted_string_length != quoted_string_length)
		{
			fprintf(stderr, "%-10.10s: EXPECTED %d\n",
			        "length",
			        (int)T->quoted_string_length);
			++exitCode;
		}
		
		if (T->consumed_8 != consumed_8)
		{
			fprintf(stderr, "%-10.10s: EXPECTED %d\n",
			        "Conumed/8",
			        (int)T->consumed_8);
			++exitCode;
		}
		
		if (T->consumed_24 != consumed_24)
		{
			fprintf(stderr, "%-10.10s: EXPECTED %d\n",
			        "Consumed/24",
			        (int)T->consumed_24);
			++exitCode;
		}
		
		if (quoted_string_length >= 0
		        && T->quoted_string_length == quoted_string_length)
		{
			if (strncmp(T->quoted_string, output, quoted_string_length))
			{
				fprintf(stderr, "%-10.10s: EXPECTED <%s>\n",
				        "String", T->quoted_string);
				++exitCode;
			}
			
			if (output[quoted_string_length] != '\0')
			{
				fprintf(stderr, "%-10.10s: MISSING NULL TERMINATION\n",
				        "String");
				++exitCode;
			}
			
			char after = output[quoted_string_length + 1];
			
			if ((after & 0xFF) != 0xFF)
			{
				fprintf(stderr, "%-10.10s: 0x%02x EXPECTED 0x%02x\n",
				        "After data", after, 0xFF);
				++exitCode;
			}
		}
	}
	
	return exitCode;
}

int main(int argc, const char **argv)
{
	int exitCode = EXIT_SUCCESS;
	
	static const httpmessage_test tests[] =
	{
		{"text_compare", test_text_compare },
		{"quoted_string", test_text_quoted_string }
	};
	
	return run_tests(tests, sizeof(tests) / sizeof(httpmessage_test),
	                 argc, argv);
}
