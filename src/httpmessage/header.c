/**************************************************************************************$
 * httpmessage
 ***************************************************************************************
 * Copyright © 2022 by Renaud Guillard (dev@nore.fr)
 * Distributed under the terms of the MIT License, see LICENSE
 ***************************************************************************************
 */

#include "httpmessage/httpmessage.h"

#include <string.h>
#include <stdlib.h>
#include <errno.h>

int httpmessage_headerfield_consume(
    const char **field,
    size_t *field_length,
    const char *text,
    size_t length)
{
	return httpmessage_token_consume(
	           field, field_length,
	           text, length);
}

void httpmessage_headervalue_init(
    httpmessage_headervalue *value)
{
	httpmessage_stringview_clear(&value->chunk);
	value->next_chunk = NULL;
}

void httpmessage_headervalue_clear(
    httpmessage_headervalue *value,
    int option_flags)
{
	httpmessage_stringview_clear(&value->chunk);
	
	if (!value->next_chunk)
	{
		return;
	}
	
	if (option_flags & HTTPMESSAGE_CLEAR_NO_FREE)
	{
		httpmessage_headervalue_clear(value->next_chunk, option_flags);
		return;
	}
	
	httpmessage_headervalue_free(&value->next_chunk);
}

httpmessage_headervalue *httpmessage_headervalue_new(void)
{
	httpmessage_headervalue *value = (httpmessage_headervalue *) malloc(sizeof(httpmessage_headervalue));
	
	if (!value)
	{
		return NULL;
	}
	
	httpmessage_headervalue_init(value);
	return value;
}

httpmessage_headervalue *httpmessage_headervalue_append_chunk(
    httpmessage_headervalue *value,
    const char *text, size_t length)
{
	httpmessage_headervalue *o = (httpmessage_headervalue *)malloc(sizeof(httpmessage_headervalue));
	o->chunk.text = text;
	o->chunk.length = length;
	value->next_chunk = o;
	return o;
}

size_t httpmessage_headervalue_total_length(const httpmessage_headervalue *value)
{
	size_t total = 0;
	
	while (value)
	{
		if (total && value->chunk.length)
		{
			++total;
		}
		
		total += value->chunk.length;
		value = value->next_chunk;
	}
	
	return total;
}

int httpmessage_headervalue_merge_chunks(
    char *output,
    size_t output_size,
    const httpmessage_headervalue *value)
{
	size_t copied = 0;
	
	if (!(output && output_size && value))
	{
		return HTTPMESSAGE_ERROR_INVALID_ARGUMENT;
	}
	
	do
	{
		size_t required = value->chunk.length;
		
		if (copied)
		{
			++required;    /* space */
		}
		
		++required; /* leading \0 */
		
		if (output_size < required)
		{
			return HTTPMESSAGE_ERROR_OVERFLOW;
		}
		
		if (copied)
		{
			*output = ' ';
			++output;
			--output_size;
			++copied;
		}
		
		memcpy(output, value->chunk.text, value->chunk.length);
		output += value->chunk.length;
		output_size -= value->chunk.length;
		copied += value->chunk.length;
		value = value->next_chunk;
	}
	while (value);
	
	*output = '\0';
	return (int)copied;
}

void httpmessage_headervalue_free(
    httpmessage_headervalue **value)
{
	if ((*value)->next_chunk)
	{
		httpmessage_headervalue_free(&(*value)->next_chunk);
	}
	
	free(*value);
	*value = NULL;
}


int httpmessage_headervalue_line_consume(
    const char **value, size_t *value_length,
    const char *text, size_t length,
    int option_flags)
{
	int consumed = 0;
	size_t vl = 0;
	*value = NULL;
	*value_length = 0;
	
	while (length && httpmessage_text_is_LWS(*text))
	{
		++consumed;
		++text;
		--length;
	}
	
	*value = text;
	
	while (length && !httpmessage_text_is_CRLF(text, length))
	{
		++vl;
		++text;
		--length;
	}
	
	if (length == 0)
	{
		if ((option_flags & HTTPMESSAGE_CONSUME_IGNORE_MISSING_CRLF) == 0)
		{
			return HTTPMESSAGE_ERROR_INCOMPLETE;
		}
	}
	
	consumed += (int) vl;
	
	/* Trim trailing white spaces */
	if (vl)
	{
		--text;
		
		while (vl)
		{
			if (!httpmessage_text_is_LWS(*text))
			{
				break;
			}
			
			--vl;
			--text;
		}
	}
	
	*value_length = vl;
	
	return consumed + (length ? 2 : 0); /* + CRLF */
}

ssize_t httpmessage_header_write_file(
    FILE *file,
    const httpmessage_header *header)
{
	ssize_t written = 0;
	const httpmessage_headervalue *value = &header->value;
	
	if (!(file && header))
	{
		return HTTPMESSAGE_ERROR_INVALID_ARGUMENT;
	}
	
	HTTPMESSAGE_STRING_WRITE_FILE(written, file, header->field)
	HTTPMESSAGE_TEXT_WRITE_FILE(written, file, ":", 1)
	
	if (value->chunk.length == 0)
	{
		HTTPMESSAGE_TEXT_WRITE_FILE(written, file, "\r\n", 2)
		return written;
	}
	
	while (value && value->chunk.length)
	{
		HTTPMESSAGE_TEXT_WRITE_FILE(written, file, " ", 1)
		HTTPMESSAGE_STRING_WRITE_FILE(written, file, value->chunk)
		HTTPMESSAGE_TEXT_WRITE_FILE(written, file, "\r\n", 2)
		value = value->next_chunk;
	}
	
	return written;
}

HMAPI ssize_t httpmessage_header_write_buffer(
    void *output, size_t output_size,
    const httpmessage_header *header)
{
	const httpmessage_headervalue *value = &header->value;
	char *o = (char *)output;
	
	if (!(output && output_size && header))
	{
		return HTTPMESSAGE_ERROR_INVALID_ARGUMENT;
	}
	
	HTTPMESSAGE_STRING_WRITE_BUFFER(o, output_size, header->field)
	HTTPMESSAGE_TEXT_WRITE_BUFFER(o, output_size, ":", 1)
	
	if (value->chunk.length == 0)
	{
		HTTPMESSAGE_TEXT_WRITE_BUFFER(o, output_size, "\r\n", 2)
		return (o - (char *)output);
	}
	
	while (value && value->chunk.length)
	{
		HTTPMESSAGE_TEXT_WRITE_BUFFER(o, output_size, " ", 1)
		HTTPMESSAGE_STRING_WRITE_BUFFER(o, output_size, value->chunk)
		HTTPMESSAGE_TEXT_WRITE_BUFFER(o, output_size, "\r\n", 2)
		value = value->next_chunk;
	}
	
	return (o - (char *)output);
}

void httpmessage_header_init(httpmessage_header *header)
{
	httpmessage_stringview_clear(&header->field);
	httpmessage_headervalue_init(&header->value);
	header->next_header = NULL;
}

int httpmessage_header_is(const httpmessage_header *header,
                          const char *name)
{
	return httpmessage_stringview_caseless_compare_text(
	           &header->field, name, strlen(name)) == 0;
}

void httpmessage_header_free(httpmessage_header **header)
{
	if ((*header)->value.next_chunk)
	{
		httpmessage_headervalue_free(&(*header)->value.next_chunk);
	}
	
	if ((*header)->next_header)
	{
		httpmessage_header_free(&(*header)->next_header);
	}
	
	free(*header);
	*header = NULL;
}

void httpmessage_header_clear(
    httpmessage_header *header,
    int option_flags)
{
	httpmessage_stringview_clear(&header->field);
	httpmessage_headervalue_clear(&header->value, option_flags);
	
	if (!header->next_header)
	{
		return;
	}
	
	if (option_flags & HTTPMESSAGE_CLEAR_NO_FREE)
	{
		httpmessage_header_clear(header->next_header, option_flags);
		return;
	}
	
	httpmessage_header_free(&header->next_header);
}

httpmessage_header *httpmessage_header_new(void)
{
	httpmessage_header *header = (httpmessage_header *)malloc(sizeof(httpmessage_header));
	
	if (!header)
	{
		return NULL;
	}
	
	httpmessage_header_init(header);
	return header;
}

size_t httpmessage_header_count(httpmessage_header *header_list)
{
	size_t c = 0;
	httpmessage_header *header = header_list;
	
	while (header)
	{
		if (header->field.length == 0)
		{
			break;
		}
		
		++c;
		header = header->next_header;
	}
	
	return c;
}

httpmessage_header *httpmessage_header_find(
    httpmessage_header *header_list,
    const char *name,
    size_t name_length)
{
	while (header_list)
	{
		if (httpmessage_stringview_caseless_compare_text(&header_list->field, name, name_length) == 0)
		{
			return header_list;
		}
		
		header_list = header_list->next_header;
	}
	
	return NULL;
}

int httpmessage_header_line_consume(
    httpmessage_header **header,
    httpmessage_header *current_header,
    const char *text, size_t length,
    int option_flags)
{
	int consumed = 0;
	int result = 0;
	const char *field = NULL;
	size_t field_length = 0;
	const char *value = NULL;
	size_t value_length = 0;
	*header = NULL;
	
	if (!(header && current_header && text))
	{
		return HTTPMESSAGE_ERROR_INVALID_ARGUMENT;
	}
	
	/* Continuation of previous header value */
	if (httpmessage_text_is_LWS(*text))
	{
		++text;
		--length;
		++consumed;
		*header = current_header;
		httpmessage_headervalue *headervalue = &current_header->value;
		
		while (headervalue->next_chunk
		        && headervalue->chunk.length)
		{
			headervalue = headervalue->next_chunk;
		}
		
		if (headervalue->chunk.length > 0)
		{
			if (option_flags & HTTPMESSAGE_NO_ALLOCATION)
			{
				return HTTPMESSAGE_ERROR_ALLOCATION;
			}
			
			headervalue->next_chunk = httpmessage_headervalue_new();
			headervalue = headervalue->next_chunk;
		}
		
		result = httpmessage_headervalue_line_consume(
		             &headervalue->chunk.text,
		             &headervalue->chunk.length,
		             text, length,
		             option_flags);
		             
		if (result <= 0)
		{
			return result;
		}
		
		return (consumed + (size_t)result);
	}
	
	/* Header field name */
	result = httpmessage_headerfield_consume(
	             &field, &field_length,
	             text, length);
	             
	if (result <= 0)
	{
		return result;
	}
	
	text += result;
	length -= (size_t)result;
	
	if (length == 0)
	{
		return HTTPMESSAGE_ERROR_INCOMPLETE;
	}
	
	/* field & value separator ":" */
	if (*text != ':')
	{
		return HTTPMESSAGE_ERROR_SYNTAX;
	}
	
	consumed += result + 1;
	++text;
	--length;
	
	/* Value */
	result = httpmessage_headervalue_line_consume(
	             &value, &value_length,
	             text, length,
	             option_flags);
	             
	if (result < 0)
	{
		return result;
	}
	
	consumed += result;
	
	*header = current_header;
	
	while ((*header)->next_header
	        && (*header)->field.length)
	{
		*header = (*header)->next_header;
	}
	
	if ((*header)->field.length)
	{
		if (option_flags & HTTPMESSAGE_NO_ALLOCATION)
		{
			return HTTPMESSAGE_ERROR_ALLOCATION;
		}
		
		(*header)->next_header = httpmessage_header_new();
		*header = (*header)->next_header;
	}
	
	(*header)->field.text = field;
	(*header)->field.length = field_length;
	(*header)->value.chunk.text = value;
	(*header)->value.chunk.length = value_length;
	
	return consumed;
}


int httpmessage_header_list_consume(
    httpmessage_header *header_list,
    const char *text, size_t length,
    int option_flags)
{
	int consumed = 0;
	httpmessage_header *new_header = NULL;
	httpmessage_header *current_header = header_list;
	httpmessage_header_clear(header_list,
	                         HTTPMESSAGE_CLEAR_NO_FREE);
	                         
	while (length && !httpmessage_text_is_CRLF(text, length))
	{
		int result = httpmessage_header_line_consume(
		                 &new_header,
		                 current_header,
		                 text, length,
		                 option_flags);
		                 
		if (result < 0)
		{
			return result;
		}
		
		if (result == 0)
		{
			break;
		}
		
		current_header = new_header;
		text += result;
		length -= (size_t)result;
		consumed += result;
	}
	
	return consumed;
}

HMAPI ssize_t httpmessage_header_list_write_file(
    FILE *file,
    const httpmessage_header *header_list)
{
	const httpmessage_header *header = header_list;
	ssize_t written = 0;
	ssize_t w = 0;
	
	while (header && header->field.length)
	{
		w = httpmessage_header_write_file(file, header);
		
		if (w < 0)
		{
			return w;
		}
		
		written += w;
		header = header->next_header;
	}
	
	return written;
}

HMAPI ssize_t httpmessage_header_list_write_buffer(
    void *output, size_t output_size,
    const httpmessage_header *header_list)
{
	const httpmessage_header *header = header_list;
	ssize_t w = 0;
	char *o = (char *)output;
	
	while (header && header->field.length)
	{
		w = httpmessage_header_write_buffer(o, output_size, header);
		
		if (w < 0)
		{
			return w;
		}
		
		o += w;
		output_size -= (size_t)w;
		header = header->next_header;
	}
	
	return (o - (char *)output);
}
