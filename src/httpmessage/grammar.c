/**************************************************************************************$
 * httpmessage
 ***************************************************************************************
 * Copyright © 2022 by Renaud Guillard (dev@nore.fr)
 * Distributed under the terms of the MIT License, see LICENSE
 ***************************************************************************************
 */

#include "httpmessage/httpmessage.h"

int httpmessage_text_is_CHAR(int c)
{
	return ((c >= 0) && (c < 128));
}

int httpmessage_text_is_UPALPHA(int c)
{
	return ((c >= 'A') && (c <= 'Z'));
}

int httpmessage_text_is_LUALPHA(int c)
{
	return ((c >= 'a') && (c <= 'z'));
}

int httpmessage_text_is_ALPHA(int c)
{
	return ((c >= 'A') && (c <= 'Z'))
	       || ((c >= 'a') && (c <= 'z'));
}

int httpmessage_text_is_LWS(int c)
{
	return (c == ' ' || c == '\t');
}

int httpmessage_text_is_DIGIT(int c)
{
	return (c >= '0' && c <= '9');
}

int httpmessage_text_is_TEXT(int c)
{
	if (httpmessage_text_is_LWS(c))
	{
		return 1;
	}
	
	return !httpmessage_text_is_CTL(c);
}

int httpmessage_text_is_CTL(int c)
{
	return ((c < 0x20) || (c == 0x7F));
}

int httpmessage_text_is_separator(int c)
{
	return (c == '(') || (c == ')') || (c == '<') || (c == '>') || (c == '@')
	       || (c == ',') || (c == ';') || (c == ':') || (c == '\\') || (c == '"')
	       || (c == '/') || (c == '[') || (c == ']') || (c == '?') || (c == '=')
	       || (c == '{') || (c == '}') || (c == ' ') || (c == '\t');
}

int httpmessage_text_is_token_char(int c)
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

ssize_t httpmessage_quoted_string_length(
    const char *text,
    size_t length)
{
	int escape = 0;
	char c;
	ssize_t quoted_string_length = 0;
	
	if (!(text && length))
	{
		return HTTPMESSAGE_ERROR_INVALID_ARGUMENT;
	}
	
	if (*text != '"')
	{
		return HTTPMESSAGE_ERROR_SYNTAX;
	}
	
	while (--length && (c = *(++text)))
	{
		if ((c == '"') && (escape == 0))
		{
			return quoted_string_length;
		}
		
		if (c == '\\')
		{
			quoted_string_length += escape;
			escape ^= 1;
			continue;
		}
		
		if (escape)
		{
			if (!httpmessage_text_is_CHAR(c))
			{
				return HTTPMESSAGE_ERROR_SYNTAX;
			}
			
			escape = 0;
			++quoted_string_length;
			continue;
		}
		
		if (!httpmessage_text_is_TEXT(c))
		{
			return HTTPMESSAGE_ERROR_SYNTAX;
		}
		
		++quoted_string_length;
	}
	
	return HTTPMESSAGE_ERROR_INCOMPLETE;
}

int httpmessage_quoted_string_consume(
    char *output,
    size_t output_size,
    const char *text,
    size_t length)
{
	int escape = 0;
	char c;
	char *s;
	int consumed = 0;
	
	if (!(text && length && output && output_size))
	{
		return HTTPMESSAGE_ERROR_INVALID_ARGUMENT;
	}
	
	s = output;
	
	if (*text != '"')
	{
		return HTTPMESSAGE_ERROR_SYNTAX;
	}
	
	++consumed;
	
	while (--length && (c = *(++text)))
	{
		++consumed;
		
		if ((c == '"') && (escape == 0))
		{
			*s = '\0';
			return consumed;
		}
		
		if (c == '\\')
		{
			if (escape)
			{
				--output_size;
				
				if (output_size == 0)
				{
					return HTTPMESSAGE_ERROR_OVERFLOW;
				}
				
				*s = c;
				++s;
			}
			
			escape ^= 1;
			continue;
		}
		
		if (escape)
		{
			if (!httpmessage_text_is_CHAR(c))
			{
				return HTTPMESSAGE_ERROR_SYNTAX;
			}
			
			--output_size;
			
			if (output_size == 0)
			{
				return HTTPMESSAGE_ERROR_OVERFLOW;
			}
			
			*s = c;
			++s;
			
			escape = 0;
			continue;
		}
		
		if (!httpmessage_text_is_TEXT(c))
		{
			return HTTPMESSAGE_ERROR_SYNTAX;
		}
		
		--output_size;
		
		if (output_size == 0)
		{
			return HTTPMESSAGE_ERROR_OVERFLOW;
		}
		
		*s = c;
		++s;
	}
	
	return HTTPMESSAGE_ERROR_INCOMPLETE;
}
