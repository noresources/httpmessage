/**************************************************************************************$
 * httpmessage
 ***************************************************************************************
 * Copyright © 2022 by Renaud Guillard (dev@nore.fr)
 * Distributed under the terms of the MIT License, see LICENSE
 ***************************************************************************************
 */

/**
 * @file grammar.h
 * @brief RFC 2616 general grammar utility
 */

#if !defined (LIBHTTPMESSAGGE_GRAMMAR_H__)
#define LIBHTTPMESSAGGE_GRAMMAR_H__

#include "httpmessage/preprocessor.h"

#include <unistd.h>

HTTPMESSAGE_C_BEGIN

/**
 * @ingroup grammar
 *
 * @brief Indicates if the given character is a character.
 *
 * @see https://datatracker.ietf.org/doc/html/rfc2616#section-2.2
 *
 * @param c Character to test
 *
 * * @return Non-zero value if @c is a character.
 */
HMAPI int httpmessage_text_is_CHAR(int c);

/**
 * @ingroup grammar
 *
 * @brief Indicates if the given character is an uppercase ASCII letter.
 *
 * @see  https://datatracker.ietf.org/doc/html/rfc2616#section-2.2
 *
 * @param c Character to test
 *
 * * @return Non-zero value if @c is an uppercase letter.
 */
HMAPI int httpmessage_text_is_UPALPHA(int c);

/**
 * @ingroup grammar
 *
 * @brief Indicates if the given character is a lowercase ASCII letter.
 *
 * @see  https://datatracker.ietf.org/doc/html/rfc2616#section-2.2
 *
 * @param c Character to test
 *
 * * @return Non-zero value if @c is lowercase letter.
 */
HMAPI int httpmessage_text_is_LUALPHA(int c);

/**
 * @ingroup grammar
 *
 * @brief Indicates if the given character is a ASCII letter.
 *
 * @see  https://datatracker.ietf.org/doc/html/rfc2616#section-2.2
 *
 * @param c Character to test
 *
 * * @return Non-zero value if @c is a letter.
 */
HMAPI int httpmessage_text_is_ALPHA(int c);

/**
 * @ingroup grammar
 *
 * @brief Indicates if the given character is a digit
 *
 * @see  https://datatracker.ietf.org/doc/html/rfc2616#section-2.2
 *
 * @param c Character to test
 *
 * @return Non-zero value if @c is a digit
 */
HMAPI int httpmessage_text_is_DIGIT(int c);

/**
 * @ingroup grammar
 *
 * @brief Indicases if the given character is a control character
 *
 * ```
 * CTL            = <any US-ASCII control character
 * ```
 *
 * @param c Character to test
 *
 * @return Non-zero value if @c is a control character
 */
HMAPI int httpmessage_text_is_CTL(int c);

/**
 * @ingroup grammar
 *
 * @brief Indicates if the given character is a linear white space
 *
 * ```
 * LWS = SP | HT
 * ```
 *
 * @param c Character to test
 * @return Non-zero value if character is a white space
 */
HMAPI int httpmessage_text_is_LWS(int c);

/**
 * @ingroup grammar
 *
 * @brief Indicates if the given text is an end of line sequence.
 *
 * ```
 * CRLF           = CR LF
 * ```
 *
 * @see https://datatracker.ietf.org/doc/html/rfc2616#section-2.2

 * @param text Text to test
 * @param length Text length
 *
 * @return Non-zero value if text starts with CRLF sequence
 */
HMAPI int httpmessage_text_is_CRLF(const char *text, size_t length);

/**
 * @ingroup grammar
 * @brief Indicates if the given character mathc the TEXT rule
 *
 * ```
 * TEXT           = <any OCTET except CTLs,
                        but including LWS>
 * ```
 *
 * @param c Character to test
 *
 * @return Non-zero value if character match the TEXT rule
 */
int httpmessage_text_is_TEXT(int c);

/**
 * @ingroup grammar
 *
 * @brief Indicates if the given character is a token separator
 *
 * ```
 * separators     = "(" | ")" | "<" | ">" | "@"
 *                     | "," | ";" | ":" | "\" | <">
 *                     | "/" | "[" | "]" | "?" | "="
 *                      | "{" | "}" | SP | HT
 * ```
 * @see https://datatracker.ietf.org/doc/html/rfc2616#section-2.2
 *
 * @param c Character to test
 *
 * @return Non-zero value if @c is a token separator
 */
HMAPI int httpmessage_text_is_separator(int c);

/**
 * @ingroup grammar
 *
 * @brief Indicates if the given character is a valid token character
 *
 * ```
 * token          = 1*<any CHAR except CTLs or separators>
 * ```
 * @see https://datatracker.ietf.org/doc/html/rfc2616#section-2.2
 *
 * @param c Character to test
 *
 * @return Non-zero value if @c is in the token character set
 */
HMAPI int httpmessage_text_is_token_char(int c);

/**
 * @ingroup grammar
 *
 * @brief Read a RFC2616 token
 *
 * ```
 * token          = 1*<any CHAR except CTLs or separators>
 * ```
 * @see https://datatracker.ietf.org/doc/html/rfc2616#section-2.2
 * @param token Output buffer
 * @param token_length Output buffer size
 * @param text Input text
 * @param length Input text length
 *
 * @return On success, the number of bytes consumed in text.
 * On error, one of the #httpmessage_result_code error codes
 *
 * @see httpmessage_result_code
 */
HMAPI int httpmessage_token_consume(const char **token,
                                    size_t *token_length,
                                    const char *text,
                                    size_t length);

/**
 * @ingroup grammar
 *
 * @brief Get the unescaped string length of a quoted string.
 *
 * ```
 * quoted-string  = ( <"> *(qdtext | quoted-pair ) <"> )
 * qdtext         = <any TEXT except <">>
 * quoted-pair    = "\" CHAR
 * ```
 *
 * @see https://datatracker.ietf.org/doc/html/rfc2616#section-2.2
 *
 * @param text Quoted string text, including leading and trailing quote.
 * @param length Quoted string length
 *
 * @return On success, length of the unescaped text.
 * On error, one of the #httpmessage_result_code error codes
 *
 * @see httpmessage_result_code
 */
HMAPI ssize_t httpmessage_quoted_string_get_length(
    const char *text,
    size_t length);

/**
 * @ingroup grammar
 *
 * @brief Unescape a quoted string text.
 *
 * ```
 * quoted-string  = ( <"> *(qdtext | quoted-pair ) <"> )
 * qdtext         = <any TEXT except <">>
 * quoted-pair    = "\" CHAR
 * ```
 *
 * @see https://datatracker.ietf.org/doc/html/rfc2616#section-2.2
 *
 * @param output Unescaped text output buffer
 * @param output_size Output buffer size
 * @param text Quoted string input text
 * @param length Ouoted string input text length
 *
 * @return On success, the number of bytes consumed in text.
 * On error, one of the #httpmessage_result_code error codes
 *
 * @see httpmessage_result_code
 */
HMAPI int httpmessage_quoted_string_consume(
    char *output,
    size_t output_size,
    const char *text,
    size_t length);

HTTPMESSAGE_C_END

#endif /* LIBHTTPMESSAGGE_GRAMMAR_H__ */

