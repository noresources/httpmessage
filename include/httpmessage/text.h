/**************************************************************************************$
 * httpmessage
 ***************************************************************************************
 * Copyright © 2022 by Renaud Guillard (dev@nore.fr)
 * Distributed under the terms of the MIT License, see LICENSE
 ***************************************************************************************
 */

/**
 * @file text.h
 * @brief Text utility
 */

#if !defined (LIBHTTPMESSAGGE_TEXT_H__)
#define LIBHTTPMESSAGGE_TEXT_H__

#include "httpmessage/preprocessor.h"

HTTPMESSAGE_C_BEGIN

#include <stddef.h>

/**
 * @ingroup text
 * @brief Reference to a text buffer
 */
typedef struct __httpmessage_stringview
{
	/** @brief First character of the string */
	const char *text;
	/** @brief Text length */
	size_t length;
} httpmessage_stringview;



/**
 * @ingroup text
 *
 * @brief Allocate and initialize a new string view
 *
 * The newly created string view points to a @c NULL reference of length 0.
 *
 * @return The newly created string view or @c NULL on error.
 */
httpmessage_stringview *httpmessage_stringview_new(void);

/**
 * @ingroup text
 * @brief Assing text to the string view
 *
 * @param string String view
 * @param text Text
 * @param length Text length
 */
HMAPI void httpmessage_stringview_assign(
    httpmessage_stringview *string,
    const char *text, size_t length);

/**
 * @ingroup text
 * @brief Reset string view
 *
 * After this call, the string view will point to a @c NULL string of length 0.
 *
 * @param string String view to reset
 */
HMAPI void httpmessage_stringview_clear(httpmessage_stringview *string);

/**
 * @ingroup text
 * @brief Free a string view.
 *
 * Release string view memory allocated with
 * @c httpmessage_stringview_new()` and set the
 * pointer to @c NULL
 *
 * @param string String view to free.
 */
HMAPI void httpmessage_stringview_free(httpmessage_stringview **string);

/**
 * @ingroup text
 * @brief Compare tw string view
 *
 * Same behavior as @c strcmp()
 *
 * @param a First string view
 * @param b Second string view
 *
 * @return < 0 if (a < 0). 0 if (a == b) and > 0 if (a > b)
 */
HMAPI int httpmessage_stringview_compare(const httpmessage_stringview *a,
        const httpmessage_stringview *b);

/**
 * @ingroup text
 * @brief Compare a string view to a text
 *
 * @param a String view to compare with text
 * @param text Text
 * @param length Text length
 *
 * @return < 0 if (string < text), 0 if (string == text) and > 0 if (string > text)
 */
HMAPI int httpmessage_stringview_compare_text(const httpmessage_stringview *a,
        const char *text, size_t length);

HTTPMESSAGE_C_END


#endif /* LIBHTTPMESSAGGE_TEXT_H__ */

