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

int httpmessage_message_http_version_consume(
    int *major_version,
    int *minor_version,
    const char *text, size_t length)
{
	int consumed = 0;
	size_t digit_count = 0;
	
	if (!(text && length))
	{
		return HTTPMESSAGE_ERROR_INVALID_ARGUMENT;
	}
	
	*major_version = 0;
	*minor_version = 0;
	
	if (length < 8)
	{
		return HTTPMESSAGE_ERROR_SYNTAX;
	}
	
	if (strncmp(text, "HTTP/", 5) != 0)
	{
		return HTTPMESSAGE_ERROR_SYNTAX;
	}
	
	text += 5;
	length -= 5;
	consumed += 5;
	
	while (length && httpmessage_text_is_DIGIT(*text))
	{
		*major_version = (*major_version * 10) + ((*text) - '0');
		++digit_count;
		++text;
		--length;
	}
	
	if (digit_count == 0
	        || *text != '.')
	{
		return HTTPMESSAGE_ERROR_SYNTAX;
	}
	
	/* "/" */
	++text;
	--length;
	consumed += (int)(digit_count + 1);
	
	digit_count = 0;
	
	while (length && httpmessage_text_is_DIGIT(*text))
	{
		*minor_version = (*minor_version * 10) + ((*text) - '0');
		++digit_count;
		++text;
		--length;
	}
	
	if (digit_count == 0)
	{
		return HTTPMESSAGE_ERROR_SYNTAX;
	}
	
	return consumed + (int)digit_count;
}

int httpmessage_request_request_uri_consume(
    httpmessage_stringview *request_uri,
    const char *text, size_t length)
{
	if (!(text && length))
	{
		return HTTPMESSAGE_ERROR_INVALID_ARGUMENT;
	}
	
	if (httpmessage_text_is_LWS(*text)
	        || httpmessage_text_is_CTL(*text)
	        || httpmessage_text_is_CRLF(text, length))
	{
		return HTTPMESSAGE_ERROR_SYNTAX;
	}
	
	httpmessage_stringview_clear(request_uri);
	
	if (*text == '*')
	{
		request_uri->text = text;
		request_uri->length = 1;
		return 1;
	}
	
	/* absoluteURI, abs_path authority */
	request_uri->text = text;
	
	while (length && !httpmessage_text_is_LWS(*text))
	{
		++request_uri->length;
		++text;
		--length;
	}
	
	if (request_uri->length == 0)
	{
		return HTTPMESSAGE_ERROR_SYNTAX;
	}
	
	return (int)request_uri->length;
}

int httpmessage_request_line_consume(
    httpmessage_stringview *method,
    httpmessage_stringview *request_uri,
    int *major_version,
    int *minor_version,
    const char *text, size_t length,
    int option_flags)
{
	int consumed = 0;
	int result = 0;
	
	if (!(text && length))
	{
		return HTTPMESSAGE_ERROR_INVALID_ARGUMENT;
	}
	
	/* Method */
	result = httpmessage_token_consume(&method->text, &method->length,
	                                   text, length);
	                                   
	if (result <= 0)
	{
		return result;
	}
	
	text += result;
	length -= (size_t)result;
	consumed += result;
	
	if (length == 0)
	{
		return HTTPMESSAGE_ERROR_INCOMPLETE;
	}
	
	/* SP */
	if (*text != ' ')
	{
		return HTTPMESSAGE_ERROR_SYNTAX;
	}
	
	++text;
	--length;
	++consumed;
	
	/* Request-URI */
	result = httpmessage_request_request_uri_consume(
	             request_uri,
	             text, length);
	             
	if (result <= 0)
	{
		return result;
	}
	
	text += result;
	length -= (size_t)result;
	consumed += result;
	
	if (length == 0)
	{
		return HTTPMESSAGE_ERROR_INCOMPLETE;
	}
	
	/* SP */
	if (*text != ' ')
	{
		return HTTPMESSAGE_ERROR_SYNTAX;
	}
	
	++text;
	--length;
	++consumed;
	
	if (length == 0)
	{
		return HTTPMESSAGE_ERROR_INCOMPLETE;
	}
	
	/* HTTP-Version */
	result = httpmessage_message_http_version_consume(
	             major_version, minor_version,
	             text, length);
	             
	if (result <= 0)
	{
		return result;
	}
	
	text += result;
	length -= (size_t)result;
	consumed += result;
	
	if (length == 0)
	{
		if ((option_flags & HTTPMESSAGE_CONSUME_IGNORE_MISSING_CRLF) == 0)
		{
			return HTTPMESSAGE_ERROR_INCOMPLETE;
		}
		
		return consumed;
	}
	
	if (!httpmessage_text_is_CRLF(text, length))
	{
		return HTTPMESSAGE_ERROR_SYNTAX;
	}
	
	return (consumed + 2);
}

int httpmessage_status_line_consume(
    int *major_version,
    int *minor_version,
    int *status_code,
    httpmessage_stringview *reason_phrase,
    const char *text, size_t length,
    int option_flags)
{
	int consumed = 0;
	int result = 0;
	int digit_count = 0;
	
	if (!(text && length))
	{
		return HTTPMESSAGE_ERROR_INVALID_ARGUMENT;
	}
	
	result = httpmessage_message_http_version_consume(
	             major_version, minor_version,
	             text, length);
	             
	if (result <= 0)
	{
		return result;
	}
	
	text += result;
	length -= (size_t)result;
	consumed += result;
	
	/* SP */
	if (length == 0)
	{
		return HTTPMESSAGE_ERROR_INCOMPLETE;
	}
	
	if (*text != ' ')
	{
		return HTTPMESSAGE_ERROR_INCOMPLETE;
	}
	
	++text;
	--length;
	++consumed;
	
	/*  Status code */
	*status_code = 0;
	
	while (length && httpmessage_text_is_DIGIT(*text))
	{
		*status_code = (*status_code * 10)
		               + (*text - '0');
		++text;
		--length;
		++digit_count;
	}
	
	if (digit_count == 0)
	{
		return HTTPMESSAGE_ERROR_SYNTAX;
	}
	
	consumed += (int)digit_count;
	
	/* Reason-Phrase  = *<TEXT, excluding CR, LF> */
	while (length
	        && httpmessage_text_is_LWS(*text))
	{
		++text;
		--length;
		++consumed;
	}
	
	reason_phrase->text = text;
	reason_phrase->length = 0;
	
	while (length
	        && httpmessage_text_is_TEXT(*text)
	        && !httpmessage_text_is_CRLF(text, length))
	{
		++text;
		--length;
		++reason_phrase->length;
	}
	
	consumed += (int)reason_phrase->length;
	
	if (length == 0)
	{
		if ((option_flags & HTTPMESSAGE_CONSUME_IGNORE_MISSING_CRLF) == 0)
		{
			return HTTPMESSAGE_ERROR_INCOMPLETE;
		}
		
		return consumed;
	}
	
	if (!httpmessage_text_is_CRLF(text, length))
	{
		return HTTPMESSAGE_ERROR_SYNTAX;
	}
	
	return (consumed + 2);
}

int httpmessage_message_get_type(
    const char *text, size_t length,
    int option_flags)
{
	int M, m, s, result;
	httpmessage_stringview a;
	httpmessage_stringview b;
	
	result = httpmessage_request_line_consume(
	             &a, &b,  &M, &m,
	             text, length,
	             option_flags);
	             
	if (result > 0)
	{
		return HTTPMESSAGE_TYPE_REQUEST;
	}
	
	result = httpmessage_status_line_consume(
	             &M, &m,
	             &s, &a,
	             text, length,
	             option_flags);
	             
	if (result > 0)
	{
		return HTTPMESSAGE_TYPE_RESPONSE;
	}
	
	return HTTPMESSAGE_TYPE_UNKNOWN;
}

void httpmessage_message_init(httpmessage_message *message)
{
	httpmessage_header_init(&message->header_list);
	httpmessage_stringview_clear(&message->body);
	message->major_version = message->minor_version = 1;
}

void httpmessage_message_clear(
    httpmessage_message *message,
    int option_flags)
{
	httpmessage_header_clear(&message->header_list, option_flags);
	httpmessage_stringview_clear(&message->body);
	message->major_version = message->minor_version = 0;
}

int httpmessage_message_append_header(
    httpmessage_message *message,
    const char *name, const char *value,
    int option_flags)
{
	httpmessage_header *header = &message->header_list;
	
	while (header->field.length && header->next_header)
	{
		header = header->next_header;
	}
	
	if (header->field.length == 0)
	{
		httpmessage_headervalue_clear(&header->value,  option_flags);
		goto httpmessage_message_append_header_ok;
	}
	
	header->next_header = httpmessage_header_new();
	
	if (!header->next_header)
	{
		return HTTPMESSAGE_ERROR_ALLOCATION;
	}
	
	header = header->next_header;
	
httpmessage_message_append_header_ok:
	httpmessage_stringview_assign(&header->field, name);
	httpmessage_stringview_assign(&header->value.chunk, value);
	return HTTPMESSAGE_OK;
}

int httpmessage_message_content_consume(
    httpmessage_message *message,
    const char *text, size_t length,
    int option_flags)
{
	int consumed = 0;
	int result = 0;
	httpmessage_header_clear(&message->header_list,
	                         /* HTTPMESSAGE_CLEAR_NO_FREE */	0);
	httpmessage_stringview_clear(&message->body);
	
	/* Headers */
	result = httpmessage_header_list_consume(
	             &message->header_list,
	             text, length);
	             
	if (result < 0)
	{
		return HTTPMESSAGE_ERROR_SYNTAX;
	}
	
	text += result;
	length -= (size_t)result;
	
	consumed += result;
	
	if (length == 0)
	{
		if ((option_flags & HTTPMESSAGE_CONSUME_IGNORE_MISSING_CRLF) == 0)
		{
			return HTTPMESSAGE_ERROR_INCOMPLETE;
		}
		
		return consumed;
	}
	
	/* CRLF line */
	
	if (!httpmessage_text_is_CRLF(text, length))
	{
		return HTTPMESSAGE_ERROR_SYNTAX;
	}
	
	text += 2;
	length -= 2;
	consumed += 2;
	
	/* Body */
	
	if (length)
	{
		message->body.text = text;
		message->body.length = length;
	}
	
	return consumed + (int)length;
}

ssize_t httpmessage_message_content_write_file(
    FILE *file,
    const httpmessage_message *message)
{
	ssize_t written = 0;
	ssize_t w;
	
	if (!(file && message))
	{
		return HTTPMESSAGE_ERROR_INVALID_ARGUMENT;
	}
	
	/* Header */
	w = httpmessage_header_list_write_file(file, &message->header_list);
	
	if (w < 0)
	{
		return w;
	}
	
	written += w;
	HTTPMESSAGE_TEXT_WRITE_FILE(written, file, "\r\n", 2)
	
	/* Body */
	HTTPMESSAGE_STRING_WRITE_FILE(written, file, message->body)
	
	return written;
}

ssize_t httpmessage_message_content_write_buffer(
    void *output, size_t output_size,
    const httpmessage_message *message)
{
	ssize_t w;
	char *o = (char *)output;
	
	if (!(output && output_size && message))
	{
		return HTTPMESSAGE_ERROR_INVALID_ARGUMENT;
	}
	
	w = httpmessage_header_list_write_buffer(o, output_size, &message->header_list);
	
	if (w < 0)
	{
		return w;
	}
	
	o += w;
	output_size -= (size_t)w;
	
	HTTPMESSAGE_TEXT_WRITE_BUFFER(o, output_size, "\r\n", 2)
	
	HTTPMESSAGE_STRING_WRITE_BUFFER(o, output_size, message->body)
	
	return (o - (char *)output);
}

void httpmessage_request_init(httpmessage_request *request)
{
	httpmessage_stringview_clear(&request->method);
	httpmessage_stringview_clear(&request->request_uri);
	httpmessage_message_init(&request->message);
}

void httpmessage_request_clear(
    httpmessage_request *request,
    int option_flags)
{
	httpmessage_message_clear(&request->message, option_flags);
	httpmessage_stringview_clear(&request->method);
	httpmessage_stringview_clear(&request->request_uri);
}

int httpmessage_request_consume(
    httpmessage_request *request,
    const char *text, size_t length,
    int option_flags)
{
	int consumed = 0;
	int result = 0;
	result = httpmessage_request_line_consume(
	             &request->method, &request->request_uri,
	             &request->message.major_version,
	             &request->message.minor_version,
	             text, length,
	             (option_flags & ~HTTPMESSAGE_CONSUME_IGNORE_MISSING_CRLF)
	         );
	         
	if (result <= 0)
	{
		return HTTPMESSAGE_ERROR_SYNTAX;
	}
	
	text += result;
	length -= (size_t)result;
	consumed += result;
	
	if (length == 0)
	{
		return HTTPMESSAGE_ERROR_INCOMPLETE;
	}
	
	result = httpmessage_message_content_consume(
	             &request->message,
	             text, length,
	             option_flags);
	             
	if (result < 0)
	{
		return HTTPMESSAGE_ERROR_SYNTAX;
	}
	
	if (result == 0)
	{
		return HTTPMESSAGE_ERROR_INCOMPLETE;
	}
	
	return consumed + result;
}

ssize_t httpmessage_request_write_file(
    FILE *file,
    const httpmessage_request *request)
{
	ssize_t written = 0;
	ssize_t w;
	
	if (!(file && request))
	{
		return HTTPMESSAGE_ERROR_INVALID_ARGUMENT;
	}
	
	/* Request line */
	HTTPMESSAGE_STRING_WRITE_FILE(written, file, request->method)
	HTTPMESSAGE_TEXT_WRITE_FILE(written, file, " ", 1)
	HTTPMESSAGE_STRING_WRITE_FILE(written, file, request->request_uri)
	HTTPMESSAGE_PRINTF_FILE(written, file, " HTTP/%d.%d\r\n",
	                        request->message.major_version, request->message.minor_version)
	                        
	w = httpmessage_message_content_write_file(file, &request->message);
	
	if (w < 0)
	{
		return w;
	}
	
	written += w;
	
	return written;
}

ssize_t httpmessage_request_write_buffer(
    void *output, size_t output_size,
    const httpmessage_request *request)
{
	char *o = (char *)output;
	ssize_t w;
	
	if (!(output && output_size && request))
	{
		return HTTPMESSAGE_ERROR_INVALID_ARGUMENT;
	}
	
	/* Request line */
	HTTPMESSAGE_STRING_WRITE_BUFFER(o, output_size, request->method)
	HTTPMESSAGE_TEXT_WRITE_BUFFER(o, output_size, " ", 1)
	HTTPMESSAGE_STRING_WRITE_BUFFER(o, output_size, request->request_uri)
	HTTPMESSAGE_PRINTF_BUFFER(o, output_size, " HTTP/%d.%d\r\n",
	                          request->message.major_version, request->message.minor_version)
	                          
	w = httpmessage_message_content_write_buffer(o, output_size,
	        &request->message);
	        
	if (w < 0)
	{
		return w;
	}
	
	o += w;
	output_size -= (size_t)w;
	
	return (o - (char *)output);
}

void httpmessage_response_init(httpmessage_response *response)
{
	response->status_code = 0;
	httpmessage_stringview_clear(&response->reason_phrase);
	httpmessage_message_init(&response->message);
}

void httpmessage_response_clear(
    httpmessage_response *response,
    int option_flags)
{
	httpmessage_message_clear(&response->message, option_flags);
	httpmessage_stringview_clear(&response->reason_phrase);
	response->status_code = 0;
}

int httpmessage_response_consume(
    httpmessage_response *response,
    const char *text, size_t length,
    int option_flags)
{
	int consumed = 0;
	int result = 0;
	
	result = httpmessage_status_line_consume(&response->message.major_version,
	         &response->message.minor_version,
	         &response->status_code,
	         &response->reason_phrase,
	         text, length,
	         (option_flags & ~HTTPMESSAGE_CONSUME_IGNORE_MISSING_CRLF)
	                                        );
	                                        
	if (result <= 0)
	{
		return HTTPMESSAGE_ERROR_SYNTAX;
	}
	
	text += result;
	length -= (size_t)result;
	consumed += result;
	
	if (length == 0)
	{
		return HTTPMESSAGE_ERROR_INCOMPLETE;
	}
	
	result = httpmessage_message_content_consume(
	             &response->message,
	             text, length,
	             option_flags);
	             
	if (result < 0)
	{
		return HTTPMESSAGE_ERROR_SYNTAX;
	}
	
	if (result == 0)
	{
		return HTTPMESSAGE_ERROR_INCOMPLETE;
	}
	
	return consumed + result;
}
