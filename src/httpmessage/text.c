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
	size_t s = (a->length < b->length) ? a->length : b->length;
	int c = strncmp(a->text, b->text, s);
	
	if (c == 0)
	{
		return (a->length < b->length);
	}
	
	return c;
}
int httpmessage_stringview_compare_text(const httpmessage_stringview *a,
                                        const char *text, size_t length)
{
	size_t s = (a->length < length) ? a->length : length;
	int c = strncmp(a->text, text, s);
	
	if (c == 0)
	{
		return (a->length < length);
	}
	
	return c;
}
