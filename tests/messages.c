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

typedef struct __http_version_test
{
	const char *input;
	int consumed;
	int major;
	int minor;
	
} __attribute__((packed))  http_version_test;

int test_http_version(int argc, const char **argv);
int test_request_line(int argc, const char **argv);
int test_status_line(int argc, const char **argv);
int test_line_type(int argc, const char **argv);
int test_first_line(int argc, const char **argv);

int test_http_version(int argc, const char **argv)
{
	(void) argc;
	(void) argv;
	int exitCode = EXIT_SUCCESS;
	size_t a;
	
	static const http_version_test tests[] =
	{
		{"HTTP/1.1", 8, 1, 1},
		{"HTTP/10.1", 9, 10, 1},
		{"HTTP/1.42", 9, 1, 42},
	};
	
	fprintf(stdout, "## %s ##############################\n",
	        "http_version");
	        
	for (a = 0; a < sizeof(tests) / sizeof(http_version_test); ++a)
	{
		const http_version_test *T = &tests[a];
		int minor_version, major_version;
		const char *text = T->input;
		size_t length = strlen(T->input);
		
		int result = httpmessage_message_http_version_consume(
		                 &major_version, &minor_version,
		                 text, length);
		                 
		fprintf(stdout, "HTTP version %10.10s -> %d.%d\n",
		        text, major_version, minor_version);
		        
		if (result != T->consumed)
		{
			fprintf(stderr, "\tConsumed = %d != %d\n", result, T->consumed);
			++exitCode;
		}
		
		if (major_version != T->major)
		{
			fprintf(stderr, "\tMajor = %d != %d\n", major_version, T->major);
			++exitCode;
		}
		
		if (minor_version != T->minor)
		{
			fprintf(stderr, "\tMinor = %d != %d\n", minor_version, T->minor);
			++exitCode;
		}
	}
	
	return exitCode;
}

/*************************************************/

typedef struct __request_line_test
{
	const char *input;
	int result;
	const char *method;
	const char *request_uri;
	int major;
	int minor;
	
} __attribute__((packed)) request_line_test;


int test_request_line(int argc, const char **argv)
{
	(void) argc;
	(void) argv;
	int exitCode = EXIT_SUCCESS;
	size_t a;
	
	static const request_line_test tests[] =
	{
		{"NOTIFY * HTTP/1.1\r\n", 19, "NOTIFY", "*", 1, 1},
		{ "POST http://foo.bar.baz HTTP/3.14\r\n", 35, "POST", "http://foo.bar.baz", 3, 14 },
		{ "GET /poil HTTP/16.17\r\n", 22, "GET", "/poil\r\n", 16, 17 }
	};
	
	fprintf(stdout, "## %s ##############################\n",
	        "request_line");
	        
	for (a = 0;
	        a < (sizeof(tests) / sizeof(request_line_test));
	        ++a)
	{
		const request_line_test *T = &tests[a];
		const char *text = T->input;
		size_t length = strlen(text);
		httpmessage_stringview method;
		httpmessage_stringview request_uri;
		int major_version;
		int minor_version;
		
		int result = httpmessage_request_line_consume(
		                 &method, &request_uri,
		                 &major_version, &minor_version,
		                 text, length,
		                 0);
		                 
		fprintf(stdout, "-- %d ----------------------------------------\n", (int)a);
		fprintf(stdout, "Request line: ");
		print_line(stdout, text, length);
		fprintf(stdout,
		        "\t%10.10s: %d\n"
		        "\t%10.10s: [%.*s]\n"
		        "\t%10.10s: [%.*s]\n"
		        "\t%10.10s: %d\n"
		        "\t%10.10s: %d\n"
		        ,
		        "Result", result,
		        "Method", (int)method.length, method.text,
		        "Resource", (int)request_uri.length, request_uri.text,
		        "Major", major_version,
		        "Minor", minor_version
		       );
		       
		if (result != T->result)
		{
			++exitCode;
			fprintf(stderr, "\t%10.10s: %d expected\n",
			        "Result", T->result);
		}
		
		if (T->result < 0)
		{
			continue;
		}
		
		if (strncmp(method.text, T->method, method.length))
		{
			++exitCode;
			fprintf(stderr, "\t%10.10s: [%s] expected\n",
			        "Method", T->method);
		}
		
		if (strncmp(request_uri.text, T->request_uri, request_uri.length))
		{
			++exitCode;
			fprintf(stderr, "\t%10.10s: [%s] expected\n",
			        "Resource", T->request_uri);
		}
		
		
		if (major_version != T->major)
		{
			++exitCode;
			fprintf(stderr, "\t%10.10s: %d expected\n",
			        "Major", T->major);
		}
		
		if (minor_version != T->minor)
		{
			++exitCode;
			fprintf(stderr, "\t%10.10s: %d expected\n",
			        "Minor", T->minor);
		}
		
	}
	
	return exitCode;
}

/**********************************************/

typedef struct __status_line_test
{
	const char *input;
	int result;
	int major;
	int minor;
	int status_code;
	const char *reason;
} status_line_test;


int test_status_line(int argc, const char **argv)
{
	(void) argc;
	(void) argv;
	int exitCode = EXIT_SUCCESS;
	size_t a;
	
	static const status_line_test tests[] =
	{
		{"HTTP/1.1 200 OK\r\n", 17, 1, 1, 200, "OK"},
		{"HTTP/1.1 200\r\n", 14, 1, 1, 200, ""},
		{"HTTP/1.1 400 Bad Request\r\n", 26, 1, 1, 400, "Bad Request"},
	};
	
	fprintf(stdout, "## %s ##############################\n",
	        "status_line");
	        
	for (a = 0;
	        a < (sizeof(tests) / sizeof(status_line_test));
	        ++a)
	{
		const status_line_test *T = &tests[a];
		const char *text = T->input;
		size_t length = strlen(text);
		httpmessage_stringview reason;
		int major_version;
		int minor_version;
		int status_code;
		
		int result = httpmessage_status_line_consume(
		                 &major_version, &minor_version,
		                 &status_code, &reason,
		                 text, length,
		                 0);
		                 
		fprintf(stdout, "-- %d ----------------------------------------\n", (int)a);
		fprintf(stdout, "Status line: ");
		print_line(stdout, text, length);
		fprintf(stdout,
		        "\t%10.10s: %d\n"
		        
		        "\t%10.10s: %d\n"
		        "\t%10.10s: %d\n"
		        "\t%10.10s: %d\n"
		        "\t%10.10s: [%.*s]\n"
		        ,
		        "Result", result,
		        "Major", major_version,
		        "Minor", minor_version,
		        "Status", status_code,
		        "Reason", (int)reason.length, reason.text
		       );
		       
		if (result != T->result)
		{
			++exitCode;
			fprintf(stderr, "\t%10.10s: %d expected\n",
			        "Result", T->result);
		}
		
		if (T->result < 0)
		{
			continue;
		}
		
		if (status_code != T->status_code)
		{
			++exitCode;
			fprintf(stderr, "\t%10.10s: %d expected\n",
			        "Status", T->status_code);
		}
		
		if (strncmp(reason.text, T->reason, reason.length))
		{
			++exitCode;
			fprintf(stderr, "\t%10.10s: [%s] expected\n",
			        "Reason", T->reason);
		}
		
		if (major_version != T->major)
		{
			++exitCode;
			fprintf(stderr, "\t%10.10s: %d expected\n",
			        "Major", T->major);
		}
		
		if (minor_version != T->minor)
		{
			++exitCode;
			fprintf(stderr, "\t%10.10s: %d expected\n",
			        "Minor", T->minor);
		}
		
	}
	
	return exitCode;
}

/**********************************************/

typedef struct __line_type_test
{
	const char *input;
	int option_flags;
	int type;
} line_type_test;


int test_line_type(int argc, const char **argv)
{
	(void) argc;
	(void) argv;
	int exitCode = EXIT_SUCCESS;
	size_t a;
	static const line_type_test tests[] =
	{
		{ "Bleh!", 0, HTTPMESSAGE_TYPE_UNKNOWN },
		{ "GET request HTTP/1.2\r\n", 0, HTTPMESSAGE_TYPE_REQUEST },
		{ "GET request HTTP/1.2", 0, HTTPMESSAGE_TYPE_UNKNOWN },
		{ "HTTP/3.14 314 Pi!\r\n", 0, HTTPMESSAGE_TYPE_RESPONSE },
		
		{ "Bleh!", HTTPMESSAGE_CONSUME_IGNORE_MISSING_CRLF, HTTPMESSAGE_TYPE_UNKNOWN },
		{ "GET request HTTP/1.2\r\n", HTTPMESSAGE_CONSUME_IGNORE_MISSING_CRLF, HTTPMESSAGE_TYPE_REQUEST },
		{ "HTTP/3.14 314 Pi!\r\n", HTTPMESSAGE_CONSUME_IGNORE_MISSING_CRLF, HTTPMESSAGE_TYPE_RESPONSE },
		
		{ "Bleh!", HTTPMESSAGE_CONSUME_IGNORE_MISSING_CRLF, -1 },
		{ "GET request HTTP/1.2", HTTPMESSAGE_CONSUME_IGNORE_MISSING_CRLF, 'q' },
		{ "HTTP/3.14 314 Pi!", HTTPMESSAGE_CONSUME_IGNORE_MISSING_CRLF, 'r' }
	};
	
	fprintf(stdout, "## %s ##############################\n",
	        "line_type");
	        
	for (a = 0;
	        a < (sizeof(tests) / sizeof(line_type_test));
	        ++a)
	{
		const line_type_test *T = &tests[a];
		const char *text = T->input;
		size_t length = strlen(text);
		
		int result = httpmessage_message_get_type(text, length,
		             T->option_flags);
		             
		fprintf(stdout, "-- %d ----------------------------------------\n", (int)a);
		fprintf(stdout, "First line: ");
		print_line(stdout, text, length);
		fprintf(stdout,
		        "\t%10.10s: 0x%02x\n"
		        "\t%10.10s: %c\n"
		        ,
		        "with flags", T->option_flags,
		        "Result", result
		       );
		       
		if (result != T->type)
		{
			++exitCode;
			fprintf(stderr, "\t%10.10s: %d expected\n",
			        "Result", T->type);
		}
	}
	
	return exitCode;
}

/***************************************************/

int test_first_line(int argc, const char **argv)
{
	(void) argc;
	(void) argv;
	int exitCode = EXIT_SUCCESS;
	
	return exitCode;
}

int main(int argc, const char **argv)
{
	static const httpmessage_test tests[] =
	{
		{"http_version", test_http_version },
		{ "status_line", test_status_line },
		{ "request_line", test_request_line },
		{ "first_line", test_first_line },
		{ "line_type", test_line_type }
	};
	
	return run_tests(tests, sizeof(tests) / sizeof(httpmessage_test),
	                 argc, argv);
}
