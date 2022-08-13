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

int httpmessage_headerfield_name_consume(
    const char **name,
    size_t *name_length,
    const char *text,
    size_t length)
{
	return httpmessage_token_consume(
	           name, name_length,
	           text, length);
}

void httpmessage_headerfield_value_init(
    httpmessage_headerfield_value *value)
{
	httpmessage_stringview_clear(&value->line);
	value->next_line = NULL;
}

void httpmessage_headerfield_value_clear(
    httpmessage_headerfield_value *value,
    int option_flags)
{
	httpmessage_stringview_clear(&value->line);
	
	if (!value->next_line)
	{
		return;
	}
	
	if (option_flags & HTTPMESSAGE_CLEAR_NO_FREE)
	{
		httpmessage_headerfield_value_clear(value->next_line, option_flags);
		return;
	}
	
	httpmessage_headerfield_value_free(&value->next_line);
}

httpmessage_headerfield_value *httpmessage_headerfield_value_new(void)
{
	httpmessage_headerfield_value *value = (httpmessage_headerfield_value *) malloc(sizeof(httpmessage_headerfield_value));
	
	if (!value)
	{
		return NULL;
	}
	
	httpmessage_headerfield_value_init(value);
	return value;
}

httpmessage_headerfield_value *httpmessage_headerfield_value_append_line(
    httpmessage_headerfield_value *value,
    const char *text, size_t length)
{
	httpmessage_headerfield_value *o = (httpmessage_headerfield_value *)malloc(sizeof(httpmessage_headerfield_value));
	o->line.text = text;
	o->line.length = length;
	value->next_line = o;
	return o;
}

size_t httpmessage_headerfield_value_total_length(const httpmessage_headerfield_value *value)
{
	size_t total = 0;
	
	while (value)
	{
		if (total && value->line.length)
		{
			++total;
		}
		
		total += value->line.length;
		value = value->next_line;
	}
	
	return total;
}

int httpmessage_headerfield_value_merge_lines(
    char *output,
    size_t output_size,
    const httpmessage_headerfield_value *value)
{
	size_t copied = 0;
	
	if (!(output && output_size && value))
	{
		return HTTPMESSAGE_ERROR_INVALID_ARGUMENT;
	}
	
	do
	{
		size_t required = value->line.length;
		
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
		
		memcpy(output, value->line.text, value->line.length);
		output += value->line.length;
		output_size -= value->line.length;
		copied += value->line.length;
		value = value->next_line;
	}
	while (value);
	
	*output = '\0';
	return (int)copied;
}

void httpmessage_headerfield_value_free(
    httpmessage_headerfield_value **value)
{
	if ((*value)->next_line)
	{
		httpmessage_headerfield_value_free(&(*value)->next_line);
	}
	
	free(*value);
	*value = NULL;
}

int httpmessage_headerfield_value_line_consume(
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

ssize_t httpmessage_headerfield_write_file(
    FILE *file,
    const httpmessage_headerfield *field)
{
	ssize_t written = 0;
	const httpmessage_headerfield_value *value = &field->value;
	
	if (!(file && field))
	{
		return HTTPMESSAGE_ERROR_INVALID_ARGUMENT;
	}
	
	HTTPMESSAGE_STRING_WRITE_FILE(written, file, field->name)
	HTTPMESSAGE_TEXT_WRITE_FILE(written, file, ":", 1)
	
	if (value->line.length == 0)
	{
		HTTPMESSAGE_TEXT_WRITE_FILE(written, file, "\r\n", 2)
		return written;
	}
	
	while (value && value->line.length)
	{
		HTTPMESSAGE_TEXT_WRITE_FILE(written, file, " ", 1)
		HTTPMESSAGE_STRING_WRITE_FILE(written, file, value->line)
		HTTPMESSAGE_TEXT_WRITE_FILE(written, file, "\r\n", 2)
		value = value->next_line;
	}
	
	return written;
}

HMAPI ssize_t httpmessage_headerfield_write_buffer(
    void *output, size_t output_size,
    const httpmessage_headerfield *field)
{
	const httpmessage_headerfield_value *value = &field->value;
	char *o = (char *)output;
	
	if (!(output && output_size && field))
	{
		return HTTPMESSAGE_ERROR_INVALID_ARGUMENT;
	}
	
	HTTPMESSAGE_STRING_WRITE_BUFFER(o, output_size, field->name)
	HTTPMESSAGE_TEXT_WRITE_BUFFER(o, output_size, ":", 1)
	
	if (value->line.length == 0)
	{
		HTTPMESSAGE_TEXT_WRITE_BUFFER(o, output_size, "\r\n", 2)
		return (o - (char *)output);
	}
	
	while (value && value->line.length)
	{
		HTTPMESSAGE_TEXT_WRITE_BUFFER(o, output_size, " ", 1)
		HTTPMESSAGE_STRING_WRITE_BUFFER(o, output_size, value->line)
		HTTPMESSAGE_TEXT_WRITE_BUFFER(o, output_size, "\r\n", 2)
		value = value->next_line;
	}
	
	return (o - (char *)output);
}

void httpmessage_headerfield_init(httpmessage_headerfield *field)
{
	httpmessage_stringview_clear(&field->name);
	httpmessage_headerfield_value_init(&field->value);
	field->next_field = NULL;
}

int httpmessage_headerfield_is(const httpmessage_headerfield *field,
                               const char *name)
{
	return httpmessage_stringview_caseless_compare_text(
	           &field->name, name, strlen(name)) == 0;
}

void httpmessage_headerfield_free(httpmessage_headerfield **field)
{
	if ((*field)->value.next_line)
	{
		httpmessage_headerfield_value_free(&(*field)->value.next_line);
	}
	
	if ((*field)->next_field)
	{
		httpmessage_headerfield_free(&(*field)->next_field);
	}
	
	free(*field);
	*field = NULL;
}

void httpmessage_headerfield_clear(
    httpmessage_headerfield *field,
    int option_flags)
{
	httpmessage_stringview_clear(&field->name);
	httpmessage_headerfield_value_clear(&field->value, option_flags);
	
	if (!field->next_field)
	{
		return;
	}
	
	if (option_flags & HTTPMESSAGE_CLEAR_NO_FREE)
	{
		httpmessage_headerfield_clear(field->next_field, option_flags);
		return;
	}
	
	httpmessage_headerfield_free(&field->next_field);
}

httpmessage_headerfield *httpmessage_headerfield_new(void)
{
	httpmessage_headerfield *header = (httpmessage_headerfield *)malloc(sizeof(httpmessage_headerfield));
	
	if (!header)
	{
		return NULL;
	}
	
	httpmessage_headerfield_init(header);
	return header;
}

size_t httpmessage_headerfield_count(httpmessage_headerfield *headerfield_list)
{
	size_t c = 0;
	httpmessage_headerfield *header = headerfield_list;
	
	while (header)
	{
		if (header->name.length == 0)
		{
			break;
		}
		
		++c;
		header = header->next_field;
	}
	
	return c;
}

httpmessage_headerfield *httpmessage_headerfield_find(
    httpmessage_headerfield *headerfield_list,
    const char *name,
    size_t name_length)
{
	while (headerfield_list)
	{
		if (httpmessage_stringview_caseless_compare_text(&headerfield_list->name, name, name_length) == 0)
		{
			return headerfield_list;
		}
		
		headerfield_list = headerfield_list->next_field;
	}
	
	return NULL;
}

int httpmessage_headerfield_line_consume(
    httpmessage_headerfield **header,
    httpmessage_headerfield *current_header,
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
		httpmessage_headerfield_value *headerfield_value = &current_header->value;
		
		while (headerfield_value->next_line
		        && headerfield_value->line.length)
		{
			headerfield_value = headerfield_value->next_line;
		}
		
		if (headerfield_value->line.length > 0)
		{
			if (option_flags & HTTPMESSAGE_NO_ALLOCATION)
			{
				return HTTPMESSAGE_ERROR_ALLOCATION;
			}
			
			headerfield_value->next_line = httpmessage_headerfield_value_new();
			headerfield_value = headerfield_value->next_line;
		}
		
		result = httpmessage_headerfield_value_line_consume(
		             &headerfield_value->line.text,
		             &headerfield_value->line.length,
		             text, length,
		             option_flags);
		             
		if (result <= 0)
		{
			return result;
		}
		
		return (consumed + (size_t)result);
	}
	
	/* Header field name */
	result = httpmessage_headerfield_name_consume(
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
	result = httpmessage_headerfield_value_line_consume(
	             &value, &value_length,
	             text, length,
	             option_flags);
	             
	if (result < 0)
	{
		return result;
	}
	
	consumed += result;
	
	*header = current_header;
	
	while ((*header)->next_field
	        && (*header)->name.length)
	{
		*header = (*header)->next_field;
	}
	
	if ((*header)->name.length)
	{
		if (option_flags & HTTPMESSAGE_NO_ALLOCATION)
		{
			return HTTPMESSAGE_ERROR_ALLOCATION;
		}
		
		(*header)->next_field = httpmessage_headerfield_new();
		*header = (*header)->next_field;
	}
	
	(*header)->name.text = field;
	(*header)->name.length = field_length;
	(*header)->value.line.text = value;
	(*header)->value.line.length = value_length;
	
	return consumed;
}


int httpmessage_headerfield_list_consume(
    httpmessage_headerfield *headerfield_list,
    const char *text, size_t length,
    int option_flags)
{
	int consumed = 0;
	httpmessage_headerfield *new_header = NULL;
	httpmessage_headerfield *current_header = headerfield_list;
	httpmessage_headerfield_clear(headerfield_list,
	                              HTTPMESSAGE_CLEAR_NO_FREE);
	                              
	while (length && !httpmessage_text_is_CRLF(text, length))
	{
		int result = httpmessage_headerfield_line_consume(
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

HMAPI ssize_t httpmessage_headerfield_list_write_file(
    FILE *file,
    const httpmessage_headerfield *headerfield_list)
{
	const httpmessage_headerfield *header = headerfield_list;
	ssize_t written = 0;
	ssize_t w = 0;
	
	while (header && header->name.length)
	{
		w = httpmessage_headerfield_write_file(file, header);
		
		if (w < 0)
		{
			return w;
		}
		
		written += w;
		header = header->next_field;
	}
	
	return written;
}

HMAPI ssize_t httpmessage_headerfield_list_write_buffer(
    void *output, size_t output_size,
    const httpmessage_headerfield *headerfield_list)
{
	const httpmessage_headerfield *header = headerfield_list;
	ssize_t w = 0;
	char *o = (char *)output;
	
	while (header && header->name.length)
	{
		w = httpmessage_headerfield_write_buffer(o, output_size, header);
		
		if (w < 0)
		{
			return w;
		}
		
		o += w;
		output_size -= (size_t)w;
		header = header->next_field;
	}
	
	return (o - (char *)output);
}
