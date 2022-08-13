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
#include <stddef.h>
#include <stdlib.h>
#include <unistd.h>
#if defined (__APPLE__)
#	include <mach/error.h>
#else
#	include <error.h>
#endif

#define MESSAGE_MAX_LENGTH 4095

void print_message(FILE *stream, const httpmessage_message *message);
void print_response(FILE *stream, const httpmessage_response *response);
void print_request(FILE *stream, const httpmessage_request *request);
int parse_response_text(const char *text, size_t length);
int parse_request_text(const char *text, size_t length);
int parse_text(const char *text, size_t length);
int parse_file(FILE *file);


void print_message(FILE *stream, const httpmessage_message *message)
{
	if (message->field_list.name.length)
	{
		const httpmessage_headerfield *field = &message->field_list;
		char value[1024];
		fprintf(stream, "\tHeaders\n");
		
		while (field && field->name.length)
		{
			httpmessage_headerfield_value_merge_lines(
			    value, sizeof(value), &field->value);
			fprintf(stream, "\t\t%-16.*s = %s\n",
			        (int)field->name.length, field->name.text,
			        value);
			field = field->next_field;
		}
	}
	
	fprintf(stream, "\tBody\n%.*s\n",
	        (int)message->body.length,
	        message->body.text
	       );
}

void print_response(FILE *stream, const httpmessage_response *response)
{
	fprintf(stream, "Response\n");
	fprintf(stream,
	        "\t%-16.16s = %d\n"
	        "\t%-16.16s = %.*s\n"
	        ,
	        "Status", response->status_code,
	        "Reason", (int)response->reason_phrase.length, response->reason_phrase.text
	       );
	print_message(stream, &response->message);
}

void print_request(FILE *stream, const httpmessage_request *request)
{
	fprintf(stream, "Request\n");
	fprintf(stream,
	        "\t%-16.16s = %.*s\n"
	        "\t%-16.16s = %.*s\n"
	        ,
	        "Method", (int)request->method.length, request->method.text,
	        "Resource", (int)request->request_uri.length, request->request_uri.text
	       );
	       
	print_message(stream, &request->message);
}

int parse_response_text(const char *text, size_t length)
{
	httpmessage_response response;
	httpmessage_response_init(&response);
	
	int result = httpmessage_response_consume(
	                 &response, text, length,
	                 0);
	                 
	if (result <= 0)
	{
		fprintf(stderr, "Failed to parse response (%d)\n", result);
		print_response(stderr, &response);
		return EXIT_FAILURE;
	}
	
	print_response(stdout, &response);
	
	return EXIT_SUCCESS;
}

int parse_request_text(const char *text, size_t length)
{
	httpmessage_request request;
	httpmessage_request_init(&request);
	int result = httpmessage_request_consume(
	                 &request, text, length,
	                 0);
	                 
	if (result <= 0)
	{
		fprintf(stderr, "Failed to parse request (%d)\n", result);
		print_request(stderr, &request);
		return EXIT_FAILURE;
	}
	
	print_request(stdout, &request);
	return EXIT_SUCCESS;
}

int parse_text(const char *text, size_t length)
{
	int type = httpmessage_message_get_type(text, length, 0);
	
	switch (type)
	{
		case HTTPMESSAGE_TYPE_RESPONSE:
			return parse_response_text(text, length);
			
		case HTTPMESSAGE_TYPE_REQUEST:
			return parse_request_text(text, length);
			
		default:
			break;
	}
	
	fprintf(stderr, "Invalid message type (%d)\n", type);
	return EXIT_FAILURE;
}

int parse_file(FILE *file)
{
	char text[ MESSAGE_MAX_LENGTH + 1];
	int fd = fileno(file);
	signed char c = EOF;
	ssize_t read_result = -1;
	size_t length = 0;
	
	if (fd <= 0)
	{
		fprintf(stderr, "Failed to get file descriptor (%d)\n", fd);
		return EXIT_FAILURE;
	}
	
	while (length < MESSAGE_MAX_LENGTH
	        && ((read_result = read(fd, &c, (size_t)1)) > (ssize_t)0)
	        && (c != EOF))
	{
		text[length] = (char)c;
		++length;
	}
	
	text[length] = '\0';
	
	if (length)
	{
		return parse_text(&text[0], length);
	}
	
	return EXIT_FAILURE;
}

int main(int argc, const char **argv)
{
	size_t command_line_message_count = 0;
	int a;
	
	for (a = 1; a < argc; ++a)
	{
		FILE *file = fopen(argv[a], "rb");
		
		if (!file)
		{
			fprintf(stderr, "Invalid file \"%s\"\n", argv[a]);
			fflush(stderr);
			return EXIT_FAILURE;
		}
		
		++command_line_message_count;
		parse_file(file);
		fclose(file);
	}
	
	if (command_line_message_count)
	{
		return EXIT_SUCCESS;
	}
	
	fflush(stdout);
	return parse_file(stdin);
	
	return EXIT_SUCCESS;
}
