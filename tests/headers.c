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

typedef struct _value_line_test
{
	const char *text;
	int option_flags;
	int result;
	const char *value;
	size_t value_length;
} value_line_test;

typedef struct __headerfield_test
{
	const char *text;
	int option_flags;
	int result;
	const char *field;
	const char *value;
} header_test;

int test_headerfield_consume(int argc, const char **argv);
int test_value_consume(int argc, const char **argv);

int test_headerfield_consume(int argc, const char **argv)
{
	(void) argc;
	(void) argv;
	size_t a = 0;
	int exitCode = EXIT_SUCCESS;
	char value[512];
	httpmessage_headerfield header;
	httpmessage_headerfield_init(&header);
	httpmessage_headerfield *current_header = &header;
	httpmessage_headerfield *new_header = NULL;
	
	
	static const header_test tests[] =
	{
		{"Foo: Bar\r\n", 0, 10, "Foo", "Bar"},
		{ "Foo : Bar\r\n", 0, HTTPMESSAGE_ERROR_SYNTAX, "", "" },
		{ "Content-Type: text/fun\r\n", 0, 24, "Content-Type", "text/fun" },
		{ "Foo:\r\n", 0,  6, "Foo", "" },
		{ "Foo:   \r\n", 0, 9, "Foo", "" },
		{ "Foo:\r\n", 0, 6, "Foo", "" },
		{ " Bar\r\n", 0, 6, "Foo", "Bar" },
		{ " 	Baz \r\n", 0, 8, "Foo", "Bar Baz" },
		
		{"Foo: Bar\r\n", HTTPMESSAGE_CONSUME_IGNORE_MISSING_CRLF, 10, "Foo", "Bar"},
		{ "Foo : Bar\r\n", HTTPMESSAGE_CONSUME_IGNORE_MISSING_CRLF, HTTPMESSAGE_ERROR_SYNTAX, "", "" },
		{ "Content-Type: text/fun\r\n", HTTPMESSAGE_CONSUME_IGNORE_MISSING_CRLF, 24, "Content-Type", "text/fun" },
		{ "Foo:\r\n", HTTPMESSAGE_CONSUME_IGNORE_MISSING_CRLF,  6, "Foo", "" },
		{ "Foo:\r\n", 0, 6, "Foo", "" },
		{ " Bar\r\n", HTTPMESSAGE_CONSUME_IGNORE_MISSING_CRLF, 6, "Foo", "Bar" },
		{ " 	Baz \r\n", HTTPMESSAGE_CONSUME_IGNORE_MISSING_CRLF, 8, "Foo", "Bar Baz" },
		{ "A:", HTTPMESSAGE_CONSUME_IGNORE_MISSING_CRLF, 2, "A", "" },
		{ "A: B ", HTTPMESSAGE_CONSUME_IGNORE_MISSING_CRLF, 5, "A", "B" },
		{ "\tC ", HTTPMESSAGE_CONSUME_IGNORE_MISSING_CRLF, 3, "A", "B C" },
		{ "\t", HTTPMESSAGE_CONSUME_IGNORE_MISSING_CRLF, 2, "A", "B C" },
		{ " D", HTTPMESSAGE_CONSUME_IGNORE_MISSING_CRLF, 2, "A", "B C D" }
	};
	
	for (a = 0; a < sizeof(tests) / sizeof(header_test); ++a)
	{
		const header_test *T = &tests[a];
		int result = 0;
		fprintf(stdout, "-- %d -------------------------------\n", (int)a);
		fprintf(stdout, "%-10.10s: ", "Header");
		print_line(stdout, T->text, strlen(T->text));
		fprintf(stdout, "%-10.10s: %d\n", "with flags", T->option_flags);
		
		new_header = NULL;
		result = httpmessage_headerfield_line_consume(
		             &new_header,
		             current_header,
		             T->text, strlen(T->text),
		             T->option_flags);
		             
		fprintf(stdout, "%10.10s: %d\n", "Result", result);
		
		if (new_header)
		{
			current_header = new_header;
		}
		
		if (T->result < 0)
		{
			if (result != T->result)
			{
				fprintf(stdout, "%10.10s: %d\n", "EXPECTED", T->result);
				exitCode = 1;
			}
			
			
			continue;
		}
		
		result = httpmessage_headerfield_value_merge_lines(
		             value, sizeof(value),
		             &current_header->value);
		             
		if (result < 0)
		{
			fprintf(stdout, "%10.10s: %d\n", "Merge", result);
			exitCode = 1;
		}
		
		fprintf(stdout, "%10.10s: %.*s\n",
		        "Field",
		        (int)current_header->name.length,
		        current_header->name.text);
		fprintf(stdout, "%10.10s: %.*s\n",
		        "Value",
		        (int) strlen(value),
		        value);
		        
		if (strcmp(T->value, value) != 0)
		{
			fprintf(stderr, "%10.10s: %.*s EXPECTED\n",
			        "Value",
			        (int) strlen(T->value),
			        T->value);
			++exitCode;
		}
	}
	
	httpmessage_headerfield_clear(&header, 0);
	
	return exitCode;
}

int test_value_consume(int argc, const char **argv)
{
	(void) argc;
	(void) argv;
	size_t a = 0;
	int exitCode = EXIT_SUCCESS;
	
	static const value_line_test tests[] =
	{
		{"foo\r\n", 0, 5, "foo", 3},
		{"foo", 0, HTTPMESSAGE_ERROR_INCOMPLETE, "foo", 3},
		{"foo bar\r\n", 0, 9, "foo bar", 7},
		{"	 foo bar\r\n", 0, 11, "foo bar", 7},
		{"foo bar  \r\n", 0, 11, "foo bar", 7},
		{ "Foo\r\n", 0, 5, "Foo", 3 },
		{ "Foo\r\nBar", 0, 5, "Foo", 3 },
		{ "  Foo\r\n", 0, 7, "Foo", 3 },
		{ "Foo 	\r\n", 0, 7, "Foo", 3 },
		
		{"foo\r\n", HTTPMESSAGE_CONSUME_IGNORE_MISSING_CRLF, 5, "foo", 3},
		{"foo", HTTPMESSAGE_CONSUME_IGNORE_MISSING_CRLF, 3, "foo", 3},
		{"foo bar\r\n", HTTPMESSAGE_CONSUME_IGNORE_MISSING_CRLF, 9, "foo bar", 7},
		{ "Foo\r\nBar", HTTPMESSAGE_CONSUME_IGNORE_MISSING_CRLF, 5, "Foo", 3 }
	};
	
	for (a = 0; a < sizeof(tests) / sizeof(value_line_test); ++a)
	{
		const value_line_test *T = &tests[a];
		const char *value;
		size_t length;
		int result;
		
		fprintf(stdout, "-- %d -------------------------\n", (int)a);
		fprintf(stdout, "%-10.10s: ", "LINE");
		print_line(stdout, T->text, strlen(T->text));
		result = httpmessage_headerfield_value_line_consume(
		             &value, &length,
		             T->text, strlen(T->text),
		             T->option_flags);
		fprintf(stdout, "%10.10s: 0x%02x\n", "with flags", T->option_flags);
		fprintf(stdout, "%10.10s: ", "VALUE");
		print_line(stdout, value, length);
		
		if (result != T->result)
		{
			fprintf(stderr, "%10.10s: %d != %d\n", "result", result, T->result);
			exitCode = EXIT_FAILURE;
		}
		
		if (T->result < 0)
		{
			continue;
		}
		
		if (length != T->value_length)
		{
			fprintf(stderr, "%10.10s: %d != %d\n", "length", (int)length, (int)T->value_length);
			exitCode = EXIT_FAILURE;
		}
		
		if (memcmp(value, T->value, T->value_length) != 0)
		{
			fprintf(stderr, "%10.10s: \n", "EXPECCTED");
			print_line(stderr, T->value, T->value_length);
			exitCode = EXIT_FAILURE;
		}
	}
	
	return exitCode;
}

int main(int argc, const char **argv)
{
	static const httpmessage_test tests[] =
	{
		{ "value_consume", test_value_consume },
		{"header_consume", test_headerfield_consume }
	};
	
	return run_tests(tests, sizeof(tests) / sizeof(httpmessage_test),
	                 argc, argv);
}
