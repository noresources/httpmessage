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
int test_storage(int argc, const char **argv);

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

/*******************************************************************/

typedef struct __storage_test
{
	const char *input_file;
	int type;
	size_t max_headerfield_count;
	size_t max_line_per_value;
	int result;
	size_t header_count;
	const char *field_name;
	const char *field_value;
} storage_test;

int test_storage(int argc, const char **argv)
{
	(void) argc;
	(void) argv;
	int exitCode = EXIT_SUCCESS;
	size_t a;
	size_t text_length;
	
	static const storage_test tests[] =
	{
		{
			"tests/data/get-lucky.request", HTTPMESSAGE_TYPE_REQUEST,
			8, 2, 158, 3,
			"X-Lyrics", "I'm a poor lonesome cowboy. I'm a long long way from home."
		},
		/** Failed because X-Lyrics header value is multipline */
		{
			"tests/data/get-lucky.request", HTTPMESSAGE_TYPE_REQUEST,
			8, 1, HTTPMESSAGE_ERROR_ALLOCATION, 3, NULL, NULL
		},
		{
			"tests/data/head-slash.response", HTTPMESSAGE_TYPE_RESPONSE,
			8, 2,
			161, 4, NULL, NULL,
		},
		{
			"tests/data/head-slash.response", HTTPMESSAGE_TYPE_RESPONSE,
			4, 2,
			161, 4,
			"content-type", "text/html; charset=iso-8859-1",
		},
		/* No enough header fields */
		{
			"tests/data/head-slash.response", HTTPMESSAGE_TYPE_RESPONSE,
			2, 2,
			HTTPMESSAGE_ERROR_ALLOCATION, 4,
			NULL, NULL
		}
	};
	
	for (a = 0; a < (sizeof(tests) / sizeof(storage_test)); ++a)
	{
		const storage_test *T = &tests[a];
		char text[4096];
		void *storage;
		httpmessage_request *request = NULL;
		httpmessage_response *response = NULL;
		httpmessage_message *message = NULL;
		FILE *file = fopen(T->input_file, "rb");
		int type;
		int result;
		int option_flags = (HTTPMESSAGE_CLEAR_NO_FREE | HTTPMESSAGE_NO_ALLOCATION);
		size_t max_headerfield_count;
		size_t max_line_per_value;
		size_t header_count;
		
		fprintf(stdout, "-- %s %d/%d ----------------------------------------\n",
		        T->input_file, (int)T->max_headerfield_count, (int)T->max_line_per_value);
		        
		if (!file)
		{
			fprintf(stderr, "warning: %s not found.\n", T->input_file);
			continue;
		}
		
		text_length = fread(text, 1, sizeof(text), file);
		
		if (!text_length)
		{
			fprintf(stderr, "warning: failed to read %s\n",
			        T->input_file);
			goto test_storage_test_loop_end;
		}
		
		type = httpmessage_message_get_type(text, text_length, 0);
		
		if (type != T->type)
		{
			fprintf(stderr, "%15.15s: %.*s, expect %.*s\n",
			        "TYPE",
			        4, (const char *)&type,
			        4, (const char *)&T->type);
			++exitCode;
			goto test_storage_test_loop_end;
		}
		
		if (type == HTTPMESSAGE_TYPE_REQUEST)
		{
			request = httpmessage_request_storage_new(
			              T->max_headerfield_count,
			              T->max_line_per_value);
			storage = request;
		}
		else if (type == HTTPMESSAGE_TYPE_RESPONSE)
		{
			response = httpmessage_response_storage_new(
			               T->max_headerfield_count,
			               T->max_line_per_value);
			storage = response;
		}
		
		if (!storage)
		{
			fprintf(stderr, "Failed to allocate storage %d/%d\n",
			        (int)T->max_headerfield_count, (int)T->max_line_per_value);
			++exitCode;
			goto test_storage_test_loop_end;
		}
		
		if (type == HTTPMESSAGE_TYPE_REQUEST)
		{
			message = &request->message;
		}
		else if (type == HTTPMESSAGE_TYPE_RESPONSE)
		{
			message = &response->message;
		}
		
		httpmessage_message_get_storage_infos(&max_headerfield_count, &max_line_per_value, message);
		
		if (max_headerfield_count != T->max_headerfield_count)
		{
			fprintf(stderr, "%15.15s: %d, expect %d\n",
			        "MAX HEADERS",
			        (int)max_headerfield_count, (int)T->max_headerfield_count);
			++exitCode;
		}
		
		if (max_line_per_value != T->max_line_per_value)
		{
			fprintf(stderr, "%15.15s: %d, expect %d\n",
			        "MAX LINES",
			        (int)max_line_per_value,
			        (int)T->max_line_per_value);
			++exitCode;
		}
		
		if (type == HTTPMESSAGE_TYPE_REQUEST)
		{
			result = httpmessage_request_consume(request,
			                                     text, text_length,
			                                     option_flags);
		}
		else if (type == HTTPMESSAGE_TYPE_RESPONSE)
		{
			result = httpmessage_response_consume(response,
			                                      text, text_length,
			                                      option_flags);
		}
		
		if (result == T->result)
		{
			fprintf(stdout, "%15.15s: %d %s\n",
			        "RESULT",
			        result,
			        ((result <= 0) ? httpmessage_result_get_text(result) : ""));
		}
		else
		{
			fprintf(stderr, "%15.15s: %d %s, expect %d %s\n",
			        "RESULT",
			        result,
			        ((result <= 0) ? httpmessage_result_get_text(result) : ""),
			        T->result,
			        ((T->result <= 0) ? httpmessage_result_get_text(T->result) : ""));
			++exitCode;
		}
		
		httpmessage_message_get_storage_infos(&max_headerfield_count, &max_line_per_value, message);
		
		if (max_headerfield_count != T->max_headerfield_count)
		{
			fprintf(stderr, "%15.15s: %d, expect %d\n",
			        "MAX HEADERS",
			        (int)max_headerfield_count, (int)T->max_headerfield_count);
			++exitCode;
		}
		
		if (max_line_per_value != T->max_line_per_value)
		{
			fprintf(stderr, "%15.15s: %d, expect %d\n",
			        "MAX LINES",
			        (int)max_line_per_value,
			        (int)T->max_line_per_value);
			++exitCode;
		}
		
		if (result <= 0)
		{
			goto test_storage_test_loop_end;
		}
		
		if (type == HTTPMESSAGE_TYPE_REQUEST)
		{
			message = &request->message;
		}
		else if (type == HTTPMESSAGE_TYPE_RESPONSE)
		{
			message = &response->message;
		}
		
		header_count = httpmessage_headerfield_count(&message->field_list);
		
		if (header_count == T->header_count)
		{
			fprintf(stdout, "%15.15s: %d\n",
			        "HEADERS",
			        (int) header_count);
		}
		else
		{
			fprintf(stderr, "%15.15s: %d, expect %d\n",
			        "HEADERS",
			        (int) header_count,
			        (int) T->header_count);
		}
		
		if (T->field_name)
		{
			size_t name_length = strlen(T->field_name);
			httpmessage_headerfield *header =
			    httpmessage_headerfield_find(&message->field_list,
			                                 T->field_name, name_length);
			                                 
			if (!header)
			{
				fprintf(stderr, "%15.15s: %s NOT FOUND\n",
				        "FIELD",
				        T->field_name);
				++exitCode;
			}
			
			if (T->field_value && header)
			{
				size_t value_length = strlen(T->field_value);
				char value[512];
				int s = httpmessage_headerfield_value_merge_lines(value, sizeof(value),
				        &header->value);
				        
				if (s >= 0 && (size_t)s == value_length)
				{
					if (strncmp(value, T->field_value, value_length) == 0)
					{
						fprintf(stdout, "%.*s: %.*s\n",
						        (int)header->name.length, header->name.text,
						        (int)value_length, value);
					}
					else
					{
						++exitCode;
					}
				}
				else
				{
					fprintf(stderr, "%15.15s: %d, expect %d\n",
					        "VALUE LENGTH",
					        s,
					        (int) value_length);
					++exitCode;
				}
			}
		}
		
test_storage_test_loop_end:
		free(request);
		request = NULL;
		free(response);
		response = NULL;
		
		if (file)
		{
			fclose(file);
		}
	}
	
	
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
		{ "line_type", test_line_type },
		{ "storage", test_storage }
	};
	
	return run_tests(tests, sizeof(tests) / sizeof(httpmessage_test),
	                 argc, argv);
}
