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

httpmessage_stringview *httpmessage_stringview_new(void)
{
	httpmessage_stringview *o = (httpmessage_stringview *)malloc(sizeof(httpmessage_stringview));
	o->text = NULL;
	o->length = 0;
	return o;
}

void httpmessage_stringview_assign(
    httpmessage_stringview *string,
    const char *text, size_t length)
{
	string->text = text;
	string->length = length;
}

void httpmessage_stringview_clear(
    httpmessage_stringview *string)
{
	string->text = NULL;
	string->length = 0;
}

void httpmessage_stringview_free(
    httpmessage_stringview **string)
{
	free(*string);
	*string = NULL;
}

int httpmessage_stringview_compare(
    const httpmessage_stringview *a,
    const httpmessage_stringview *b)
{
	return httpmessage_stringview_compare_text(a, b->text, b->length);
}
int httpmessage_stringview_compare_text(const httpmessage_stringview *a,
                                        const char *text, size_t length)
{
	size_t comparable_length = (a->length < length) ? a->length : length;
	int comparison = strncmp(a->text, text, comparable_length);
	
	if (comparison == 0)
	{
		return (a->length - length);
	}
	
	return comparison;
}

int httpmessage_stringview_caseless_compare(
    const httpmessage_stringview *a,
    const httpmessage_stringview *b)
{
	return httpmessage_stringview_caseless_compare_text(a, b->text, b->length);
}

int httpmessage_stringview_caseless_compare_text(const httpmessage_stringview *v,
        const char *text, size_t length)
{
	size_t comparable_length = (v->length < length) ? v->length : length;
	size_t a;
	char ca, cb;
	char to_lower = ('a' - 'A');
	
	for (a = 0; a < comparable_length; ++a)
	{
		ca = v->text[a];
		cb = text[a];
		
		if (ca == cb)
		{
			continue;
		}
		
		if ((ca >= 'A') && (ca <= 'Z'))
		{
			if ((cb >= 'a') && (cb <= 'z')
			        && ((ca + to_lower) == cb))
			{
				continue;
			}
		}
		else if ((ca >= 'a') && (ca <= 'z'))
		{
			if ((cb >= 'A') && (cb <= 'Z')
			        && ((cb + to_lower) == ca))
			{
				continue;
			}
		}
		
		return ca - cb;
	}
	
	return v->length - length;
}
