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
} compare_test;

int test_text_compare(int argc, const char **argv);
int test_quoted_string(int argc, const char **argv);
int test_int(int argc, const char **argv);
int test_token_consume(int argc, const char **argv);;
int test_is_char(int argc, const char **argv);

int test_text_compare(int argc, const char **argv)
{
	(void) argc;
	(void) argv;
	int exit_code = EXIT_SUCCESS;
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
		        
		if (!same_sign(T->compared, compared))
		{
			++exit_code;
			fprintf(stdout, " <=-> expected %d\n", T->compared);
		}
		
		fprintf(stdout, "\n");
		
		fprintf(stdout, "%-10.10s: %d",
		        "Caseless", caseless_compared);
		        
		if (!same_sign(T->caseless_compared, caseless_compared))
		{
			++exit_code;
			fprintf(stdout, " <=-> expected %d\n", T->caseless_compared);
		}
		
		fprintf(stdout, "\n");
		
		
	}
	
	return exit_code;
}

typedef struct __quoted_string_test
{
	const char *input;
	ssize_t quoted_string_length;
	int consumed_8;
	int consumed_24;
	const char *quoted_string;
} quoted_string_test;

int test_quoted_string(int argc, const char **argv)
{
	int exit_code = EXIT_SUCCESS;
	size_t a;
	
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
		
		for (a = 1; a < (size_t)argc; ++a)
		{
			const char *text = argv[a];
			int consumed;
			ssize_t quoted_string_length;
			size_t length = strlen(text);
			
			quoted_string_length = httpmessage_quoted_string_length(text, length);
			
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
		        
		quoted_string_length = httpmessage_quoted_string_length(text, length);
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
			++exit_code;
		}
		
		if (T->consumed_8 != consumed_8)
		{
			fprintf(stderr, "%-10.10s: EXPECTED %d\n",
			        "Conumed/8",
			        (int)T->consumed_8);
			++exit_code;
		}
		
		if (T->consumed_24 != consumed_24)
		{
			fprintf(stderr, "%-10.10s: EXPECTED %d\n",
			        "Consumed/24",
			        (int)T->consumed_24);
			++exit_code;
		}
		
		if (quoted_string_length >= 0
		        && T->quoted_string_length == quoted_string_length)
		{
			if (strncmp(T->quoted_string, output, (size_t)quoted_string_length))
			{
				fprintf(stderr, "%-10.10s: EXPECTED <%s>\n",
				        "String", T->quoted_string);
				++exit_code;
			}
			
			if (output[quoted_string_length] != '\0')
			{
				fprintf(stderr, "%-10.10s: MISSING NULL TERMINATION\n",
				        "String");
				++exit_code;
			}
			
			char after = output[quoted_string_length + 1];
			
			if ((after & 0xFF) != 0xFF)
			{
				fprintf(stderr, "%-10.10s: 0x%02x EXPECTED 0x%02x\n",
				        "After data", after, 0xFF);
				++exit_code;
			}
		}
	}
	
	return exit_code;
}

typedef struct __int_test
{
	const char *text;
	size_t length;
	int result;
	int value;
} int_test;

int test_int(int argc, const char **argv)
{
	int exit_code = EXIT_SUCCESS;
	size_t a;
	
	static const int_test tests[] =
	{
		{ NULL, 2, HTTPMESSAGE_ERROR_INVALID_ARGUMENT, 0x0def },
		{ "", 0, HTTPMESSAGE_ERROR_INVALID_ARGUMENT, 0x0def },
		
		{ "-", 1, HTTPMESSAGE_ERROR_INCOMPLETE, 0x0def },
		{ "a", 1, HTTPMESSAGE_ERROR_SYNTAX, 0x0def },
		
		{ "000", 3, 3, 0 },
		{ "-00", 3, 3, 0 },
		
		
		{ "123", 3, 3, 123 },
		{ "067", 3, 3, 67 },
		{ "-123", 4, 4, -123 },
		{ "456ignored", 9, 3, 456 }
	};
	
	for (a = 0; a < sizeof (tests) / sizeof (const int_test); ++a)
	{
		const int_test *T = &tests[a];
		int value;
		int result;
		
		fprintf(stdout, "### %d \"%.*s\"\n", (int)(a + 1),
		        (T->text ? (int)T->length : 0), (T->text ? T->text : ""));
		        
		result = httpmessage_int_consume(&value,  T->text,  T->length);
		
		if (result != T->result)
		{
			fprintf(stderr, "%-15.15s: %d, expected %d\n",
			        "RESULT", result, T->result);
			++exit_code;
		}
		
		if (result != HTTPMESSAGE_OK)
		{
			continue;
		}
		
		if (value != T->value)
		{
			++exit_code;
		}
	}
	
	
	return exit_code;
}

int test_token_consume(int argc, const char **argv);

int test_is_char(int argc, const char **argv)
{
	(void) argc;
	(void) argv;
	int exit_code = EXIT_SUCCESS;
	
	fprintf(stdout, "-- is_CHAR ----------------------------\n");
	
	if (!httpmessage_text_is_CHAR('a'))
	{
		++exit_code;
		fprintf(stderr, "is_CHAR 'a' should be true\n");
	}
	
	if (!httpmessage_text_is_CHAR('\0'))
	{
		++exit_code;
		fprintf(stderr, "is_CHAR '\\0' should be true\n");
	}
	
	if (!httpmessage_text_is_CHAR('\x7F'))
	{
		++exit_code;
		fprintf(stderr, "is_CHAR 0x7F should be true\n");
	}
	
	if (httpmessage_text_is_CHAR('\x80'))
	{
		++exit_code;
		fprintf(stderr, "is_CHAR 0x80 should be false\n");
	}
	
	if (httpmessage_text_is_CHAR(-1))
	{
		++exit_code;
		fprintf(stderr, "is_CHAR -1 should be false\n");
	}
	
	fprintf(stdout, "-- is_UPALPHA -------------------------\n");
	
	if (!httpmessage_text_is_UPALPHA('A'))
	{
		++exit_code;
		fprintf(stderr, "is_UPALPHA 'A' should be true\n");
	}
	
	if (!httpmessage_text_is_UPALPHA('Z'))
	{
		++exit_code;
		fprintf(stderr, "is_UPALPHA 'Z' should be true\n");
	}
	
	if (httpmessage_text_is_UPALPHA('a'))
	{
		++exit_code;
		fprintf(stderr, "is_UPALPHA 'a' should be false\n");
	}
	
	if (httpmessage_text_is_UPALPHA('0'))
	{
		++exit_code;
		fprintf(stderr, "is_UPALPHA '0' should be false\n");
	}
	
	fprintf(stdout, "-- is_LUALPHA -------------------------\n");
	
	if (!httpmessage_text_is_LUALPHA('a'))
	{
		++exit_code;
		fprintf(stderr, "is_LUALPHA 'a' should be true\n");
	}
	
	if (!httpmessage_text_is_LUALPHA('z'))
	{
		++exit_code;
		fprintf(stderr, "is_LUALPHA 'z' should be true\n");
	}
	
	if (httpmessage_text_is_LUALPHA('A'))
	{
		++exit_code;
		fprintf(stderr, "is_LUALPHA 'A' should be false\n");
	}
	
	if (httpmessage_text_is_LUALPHA('0'))
	{
		++exit_code;
		fprintf(stderr, "is_LUALPHA '0' should be false\n");
	}
	
	fprintf(stdout, "-- is_ALPHA ---------------------------\n");
	
	if (!httpmessage_text_is_ALPHA('a'))
	{
		++exit_code;
		fprintf(stderr, "is_ALPHA 'a' should be true\n");
	}
	
	if (!httpmessage_text_is_ALPHA('Z'))
	{
		++exit_code;
		fprintf(stderr, "is_ALPHA 'Z' should be true\n");
	}
	
	if (httpmessage_text_is_ALPHA('0'))
	{
		++exit_code;
		fprintf(stderr, "is_ALPHA '0' should be false\n");
	}
	
	if (httpmessage_text_is_ALPHA('!'))
	{
		++exit_code;
		fprintf(stderr, "is_ALPHA '!' should be false\n");
	}
	
	fprintf(stdout, "-- is_DIGIT ---------------------------\n");
	
	if (!httpmessage_text_is_DIGIT('0'))
	{
		++exit_code;
		fprintf(stderr, "is_DIGIT '0' should be true\n");
	}
	
	if (!httpmessage_text_is_DIGIT('9'))
	{
		++exit_code;
		fprintf(stderr, "is_DIGIT '9' should be true\n");
	}
	
	if (httpmessage_text_is_DIGIT('a'))
	{
		++exit_code;
		fprintf(stderr, "is_DIGIT 'a' should be false\n");
	}
	
	if (httpmessage_text_is_DIGIT('/'))
	{
		++exit_code;
		fprintf(stderr, "is_DIGIT '/' should be false\n");
	}
	
	fprintf(stdout, "-- is_CTL -----------------------------\n");
	
	if (!httpmessage_text_is_CTL('\0'))
	{
		++exit_code;
		fprintf(stderr, "is_CTL '\\0' should be true\n");
	}
	
	if (!httpmessage_text_is_CTL('\x1F'))
	{
		++exit_code;
		fprintf(stderr, "is_CTL 0x1F should be true\n");
	}
	
	if (!httpmessage_text_is_CTL('\x7F'))
	{
		++exit_code;
		fprintf(stderr, "is_CTL 0x7F should be true\n");
	}
	
	if (httpmessage_text_is_CTL(' '))
	{
		++exit_code;
		fprintf(stderr, "is_CTL ' ' should be false\n");
	}
	
	if (httpmessage_text_is_CTL('a'))
	{
		++exit_code;
		fprintf(stderr, "is_CTL 'a' should be false\n");
	}
	
	fprintf(stdout, "-- is_LWS -----------------------------\n");
	
	if (!httpmessage_text_is_LWS(' '))
	{
		++exit_code;
		fprintf(stderr, "is_LWS ' ' should be true\n");
	}
	
	if (!httpmessage_text_is_LWS('\t'))
	{
		++exit_code;
		fprintf(stderr, "is_LWS '\\t' should be true\n");
	}
	
	if (httpmessage_text_is_LWS('\r'))
	{
		++exit_code;
		fprintf(stderr, "is_LWS '\\r' should be false\n");
	}
	
	if (httpmessage_text_is_LWS('\n'))
	{
		++exit_code;
		fprintf(stderr, "is_LWS '\\n' should be false\n");
	}
	
	if (httpmessage_text_is_LWS('a'))
	{
		++exit_code;
		fprintf(stderr, "is_LWS 'a' should be false\n");
	}
	
	fprintf(stdout, "-- is_CRLF ----------------------------\n");
	
	if (!httpmessage_text_is_CRLF("\r\n", 2))
	{
		++exit_code;
		fprintf(stderr, "is_CRLF \"\\r\\n\" len=2 should be true\n");
	}
	
	if (httpmessage_text_is_CRLF("\r\n", 1))
	{
		++exit_code;
		fprintf(stderr, "is_CRLF \"\\r\\n\" len=1 should be false\n");
	}
	
	if (httpmessage_text_is_CRLF("\r ",  2))
	{
		++exit_code;
		fprintf(stderr, "is_CRLF \"\\r \" should be false\n");
	}
	
	if (httpmessage_text_is_CRLF("\n\r", 2))
	{
		++exit_code;
		fprintf(stderr, "is_CRLF \"\\n\\r\" should be false\n");
	}
	
	fprintf(stdout, "-- is_separator -----------------------\n");
	{
		const char *separators = "()<>@,;:\\\"/[]?={} \t";
		size_t i;
		
		for (i = 0; separators[i]; ++i)
		{
			if (!httpmessage_text_is_separator(separators[i]))
			{
				++exit_code;
				fprintf(stderr, "is_separator '%c' (0x%02x) should be true\n",
				        separators[i], (unsigned char)separators[i]);
			}
		}
	}
	
	if (httpmessage_text_is_separator('a'))
	{
		++exit_code;
		fprintf(stderr, "is_separator 'a' should be false\n");
	}
	
	if (httpmessage_text_is_separator('0'))
	{
		++exit_code;
		fprintf(stderr, "is_separator '0' should be false\n");
	}
	
	fprintf(stdout, "-- is_token_char ----------------------\n");
	
	if (!httpmessage_text_is_token_char('a'))
	{
		++exit_code;
		fprintf(stderr, "is_token_char 'a' should be true\n");
	}
	
	if (!httpmessage_text_is_token_char('Z'))
	{
		++exit_code;
		fprintf(stderr, "is_token_char 'Z' should be true\n");
	}
	
	if (!httpmessage_text_is_token_char('0'))
	{
		++exit_code;
		fprintf(stderr, "is_token_char '0' should be true\n");
	}
	
	if (!httpmessage_text_is_token_char('-'))
	{
		++exit_code;
		fprintf(stderr, "is_token_char '-' should be true\n");
	}
	
	if (httpmessage_text_is_token_char('('))
	{
		++exit_code;
		fprintf(stderr, "is_token_char '(' should be false\n");
	}
	
	if (httpmessage_text_is_token_char(' '))
	{
		++exit_code;
		fprintf(stderr, "is_token_char ' ' should be false\n");
	}
	
	if (httpmessage_text_is_token_char('\r'))
	{
		++exit_code;
		fprintf(stderr, "is_token_char '\\r' should be false\n");
	}
	
	return exit_code;
}

typedef struct __token_test
{
	const char *text;
	size_t length;
	ssize_t result;
	size_t token_length;
} token_test;

int test_token_consume(int argc, const char **argv)
{
	(void) argc;
	(void) argv;
	int exit_code = EXIT_SUCCESS;
	size_t a;
	
	static const token_test tests[] =
	{
		{ NULL, 1, HTTPMESSAGE_ERROR_INVALID_ARGUMENT, 0 },
		{ "", 0, HTTPMESSAGE_ERROR_INVALID_ARGUMENT, 0 },
		{ "(", 1, HTTPMESSAGE_ERROR_SYNTAX, 0 },
		{ "a", 1, 1, 1 },
		{ "Foo", 3, 3, 3 },
		{ "Content-Type", 12, 12, 12 },
		{ "Content-Type:", 13, 12, 12 },
	};
	
	for (a = 0; a < sizeof(tests) / sizeof(token_test); ++a)
	{
		const token_test *T = &tests[a];
		const char *token = NULL;
		size_t token_length = 0;
		ssize_t result;
		
		fprintf(stdout, "-- %d \"%.*s\" ----------------------------\n",
		        (int)a,
		        (T->text ? (int)T->length : 0),
		        (T->text ? T->text : ""));
		        
		result = httpmessage_token_consume(&token, &token_length,
		                                   T->text, T->length);
		                                   
		if (result != T->result)
		{
			++exit_code;
			fprintf(stderr, "%10.10s: %d, expected %d\n",
			        "result", (int)result, (int)T->result);
		}
		
		if (T->result < 0)
		{
			continue;
		}
		
		if (token_length != T->token_length)
		{
			++exit_code;
			fprintf(stderr, "%10.10s: %d, expected %d\n",
			        "length", (int)token_length, (int)T->token_length);
		}
		
		if (token && T->token_length
		        && strncmp(token, T->text, T->token_length) != 0)
		{
			++exit_code;
			fprintf(stderr, "%10.10s: [%.*s] expected [%.*s]\n",
			        "token",
			        (int)token_length, token,
			        (int)T->token_length, T->text);
		}
	}
	
	return exit_code;
}

int main(int argc, const char **argv)
{
	static const httpmessage_test tests[] =
	{
		{"text_compare", test_text_compare },
		{"quoted_string", test_quoted_string },
		{"int", test_int },
		
		/* Written by Claude code */
		{"is_char", test_is_char },
		{"token_consume", test_token_consume }
	};
	
	return run_tests(tests, sizeof(tests) / sizeof(httpmessage_test),
	                 argc, argv);
}
