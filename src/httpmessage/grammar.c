/**************************************************************************************$
 * httpmessage
 ***************************************************************************************
 * Copyright © 2022 by Renaud Guillard (dev@nore.fr)
 * Distributed under the terms of the MIT License, see LICENSE
 ***************************************************************************************
 */

#include "httpmessage/httpmessage.h"

int httpmessage_text_is_LWS(char c)
{
	return (c == ' ' || c == '\t');
}

int httpmessage_text_is_DIGIT(char c)
{
	return (c >= '0' && c <= '9');
}

int httpmessage_text_is_TEXT(char c)
{
	if (httpmessage_text_is_LWS(c))
	{
		return 1;
	}
	
	return !httpmessage_text_is_CTL(c);
}

int httpmessage_text_is_CTL(char c)
{
	return ((c < 0x20) || (c == 0x7F));
}

int httpmessage_text_is_separator(char c)
{
	return (c == '(') || (c == ')') || (c == '<') || (c == '>') || (c == '@')
	       || (c == ',') || (c == ';') || (c == ':') || (c == '\\') || (c == '"')
	       || (c == '/') || (c == '[') || (c == ']') || (c == '?') || (c == '=')
	       || (c == '{') || (c == '}') || (c == ' ') || (c == '\t');
}

int httpmessage_text_is_token_char(char c)
{
	return !(httpmessage_text_is_separator(c)
	         || httpmessage_text_is_CTL(c));
}


int httpmessage_text_is_CRLF(const char *text, size_t length)
{
	return (length >= 2
	        && (*text == '\r')
	        && (*(text + 1) == '\n')
	       );
}

int httpmessage_token_consume(const char **token,
                              size_t *token_length,
                              const char *text,
                              size_t length)
{
	if (!(token && token_length && text && length))
	{
		return HTTPMESSAGE_ERROR_INVALID_ARGUMENT;
	}
	
	*token = NULL;
	*token_length = 0;
	
	if (!httpmessage_text_is_token_char(*text))
	{
		return HTTPMESSAGE_ERROR_SYNTAX;
	}
	
	*token = text;
	++text;
	++(*token_length);
	--length;
	
	while (length && httpmessage_text_is_token_char(*text))
	{
		++text;
		++(*token_length);
		--length;
	}
	
	return (int) * token_length;
}

