/**************************************************************************************$
 * httpmessage
 ***************************************************************************************
 * Copyright © 2022 by Renaud Guillard (dev@nore.fr)
 * Distributed under the terms of the MIT License, see LICENSE
 ***************************************************************************************
 */

/**
 * @file header.h
 * @brief HTTP header parsing and manipulation
 */

#if !defined (LIBHTTPMESSAGGE_HEADER_H__)
#define LIBHTTPMESSAGGE_HEADER_H__

#include "httpmessage/preprocessor.h"
#include "httpmessage/text.h"

HTTPMESSAGE_C_BEGIN

#include <stddef.h>
#include <stdio.h>
#include <unistd.h>


/**
 * @ingroup header
 *
 * @brief Headef field name text reference
 */
typedef httpmessage_stringview httpmessage_headerfield_name;



/**
 * @ingroup header
 *
 * @brief Read the header field name
 *
 * Alias of httpmessage_token_consume()
 *
 * @param name Output field
 * @param name_length Output field length
 * @param text Input text
 * @param length Input text length
 *
 * @return On success, number of bytes of @c text consumed.
 * On error, one of httpmessage_result_code
 *
 * @see httpmessage_result_code
 */
HMAPI int httpmessage_headerfield_name_consume(
    const char **name,
    size_t *name_length,
    const char *text,
    size_t length);

/**
 * @ingroup header
 *
 * @brief HTTP header field value text reference
 *
 * @extends httpmessage_stringview
 *
 * A headef value MAY be splittd on multiple line.
 * In this case, the @c next_line member of @c httpmessage_headerfield_value
 * will point on another httpmessage_headerfield_value representing
 * the next line of text for the value.
 *
 */
typedef struct __httpmessage_headerfield_value
{
	/** First line of value */
	httpmessage_stringview line;
	
	/** Reference the the next header field value line */
	struct __httpmessage_headerfield_value *next_line;
} httpmessage_headerfield_value;

/**
 * @brief Initialize a header field value
 *
 * Ensure all structure member are initialize
 * with a meaningful value.
 *
 * @param value Header field value to initialize
 * @param option_flags Option flags. Supported flags are:
 * - @c ::HTTPMESSAGE_CLEAR_NO_FREE
 *
 * @see httpmessage_option_flags
 */
HMAPI void httpmessage_headerfield_value_clear(
    httpmessage_headerfield_value *value,
    int option_flags);

/**
 * @ingroup header
 *
 * @brief Allocate and initialize a new header value
 *
 * @return The new header value or @c NULL on error
 */
HMAPI httpmessage_headerfield_value *httpmessage_headerfield_value_new(void);


/**
 * @ingroup header
 * @brief Initialize a header value
 *
 * @param value Instance to initialize
 */
HMAPI void httpmessage_headerfield_value_init(
    httpmessage_headerfield_value *value);

/**
 * @ingroup header
 *
 * @brief Append a new line of data to the given header value
 * @param value Header value that will receive the line
 * @param text Text to add
 * @param length Text length
 *
 * @return The newly created httpmessage_headerfield_value
 */
HMAPI httpmessage_headerfield_value *httpmessage_headerfield_value_append_line(
    httpmessage_headerfield_value *value,
    const char *text, size_t length);

/**
 * @ingroup header
 *
 * @brief Get the total length of the header value text
 *
 * @param value Header value
 * @return Sum of all header value line lenght
 */
HMAPI size_t httpmessage_headerfield_value_total_length(const httpmessage_headerfield_value *value);

/**
 * @ingroup header
 *
 * @brief Copy the referenced header value lines to a contiguous text buffer.
 *
 * @param output Output buffer
 * @param output_size Output buffer size
 * @param value Input header value
 * @return On success, the number of bytes copied to @c output (excluding the NULL termination character).
 * On error, one of the httpmessage_result_code codes
 *
 * @see httpmessage_result_code
 */
HMAPI int httpmessage_headerfield_value_merge_lines(
    char *output,
    size_t output_size,
    const httpmessage_headerfield_value *value);

/**
 * @ingroup header
 *
 * @brief Free header value and following lines
 *
 * @param value Header value to free
 */
HMAPI void httpmessage_headerfield_value_free(
    httpmessage_headerfield_value **value);

/**
 * @ingroup header
 *
 * @brief Read header value content until the end of the current line
 *
 * @param value Pointer that will hold a reference to the first meaningful byte of the header value
 * @param value_length Pointer that will hold the trimmed header value length
 * @param text Input text
 * @param length Input text length
 * @param option_flags Option flags. Supported flags are
 * - @c ::HTTPMESSAGE_CONSUME_IGNORE_MISSING_CRLF
 *
 * @return On success, the number of bytes of @c text consumed. This MAY be greated than the value
 * stored in @c value_length since leading white space and trailing CRLF and white space are not counted in @c value_length.
 * On error, one of the httpmessage_result_code
 *
 * @see httpmessage_result_code
 * @see httpmessage_option_flags
 */
HMAPI int httpmessage_headerfield_value_line_consume(
    const char **value, size_t *value_length,
    const char *text, size_t length,
    int option_flags);

/**
 * @ingroup header
 *
 * @brief HTTP header field and value
 *
 */
typedef struct __httpmessage_headerfield
{
	/** Header field name */
	httpmessage_headerfield_name name;
	/** Header field value */
	httpmessage_headerfield_value value;
	/** The following header */
	struct __httpmessage_headerfield *next_field;
	
} httpmessage_headerfield;


/**
 * @ingroup header
 *
 * @brief Allocate and initialize a new header object
 *
 * @return The new header object or NULL on error.
 */
HMAPI httpmessage_headerfield *httpmessage_headerfield_new(void);

/**
 * @ingroup header
 *
 * @brief Initialize header members
 *
 * @param field Header structure to initialize
 */
HMAPI void httpmessage_headerfield_init(httpmessage_headerfield *field);

/**
 * @ingroup header
 *
 * @brief Free header memory
 *
 * @param field Header to free
 */
HMAPI void httpmessage_headerfield_free(httpmessage_headerfield **field);

/**
 * @ingroup header
 *
 * @brief Initialize a header descriptor
 *
 * @param field Header descriptor instance
 */
HMAPI void httpmessage_headerfield_init(httpmessage_headerfield *field);


/**
 * @ingroup header
 * @brief Indicates if the given header field name match the expected name.
 *
 * String comparison is case instensitive.
 *
 * @param field Header to test field name
 * @param name Name to match. The string MUST be null-terminated
 *
 * @return Non-zero value if the header field name match @c name
 */
HMAPI int httpmessage_headerfield_is(const httpmessage_headerfield *field,
                                     const char *name);

/**
 * @ingroup header
 *
 * @brief Clear header member to their initial state.
 *
 * Except if @c ::HTTPMESSAGE_CLEAR_NO_FREE option is set,
 * all dynamically allocated members are freed.
 *
 * @param field Header to clear
 * @param option_flags Option flags. Supported flags are:
 * - @c ::HTTPMESSAGE_CLEAR_NO_FREE
 *
 * @see httpmessage_option_flags
 */
HMAPI void httpmessage_headerfield_clear(
    httpmessage_headerfield *field,
    int option_flags);

/**
 * @ingroup header
 *
 * @brief Get the number of valid headers in the given header list
 *
 * @param headerfield_list Header list
 * @return Number of element in @c headerfield_list
 */
HMAPI  size_t httpmessage_headerfield_count(httpmessage_headerfield *headerfield_list);

/**
 * @ingroup header
 *
 * @brief Find the first header field matching the given field name.
 *
 * @param headerfield_list Header field list.
 * @param name Header field name to find.
 * @param name_length Header field name length.
 *
 * @return Pointer to the first header field with the given field name
 * or @c NULL if none of the header fields in the list have the exepected field name.
 */
HMAPI httpmessage_headerfield *httpmessage_headerfield_find(
    httpmessage_headerfield *headerfield_list,
    const char *name,
    size_t name_length);

/**
 * @ingroup header
 *
 * @brief Read header field name and value on a single line
 *  *
 * If the current line starts by a header field name,
 * a new header is created assigned as next header of @c current_header
 * and @c header will point to it.
 *
 * If the current line starts with a white psace, the content of the
 * line is added the the @c current_header value
 * and @c header will point to @c current_header
 *
 * @param field The new header
 * @param current_header The current header
 * @param text Input text
 * @param length Input text length
 * @param option_flags Option flags. Supported flags are:
 * - ::HTTPMESSAGE_CONSUME_IGNORE_MISSING_CRLF
 * - ::HTTPMESSAGE_CLEAR_NO_FREE
 * - ::HTTPMESSAGE_NO_ALLOCATION
 *
 * @return On success, the numbre of bytes consumed (the current line length).
 * On error, one of httpmessage_result_code
 *
 *@see httpmessage_result_code
 * @see httpmessage_option_flags
 */
HMAPI int httpmessage_headerfield_line_consume(
    httpmessage_headerfield **field,
    httpmessage_headerfield *current_header,
    const char *text, size_t length,
    int option_flags);

/**
 * @ingroup header
 * @brief Write a HTTP header line to a file
 *
 * @param file Output file
 * @param field Header to write
 *
 * @attention NULL termination character may not be written at end of string
 * if the buffer is too small.
 *
 * @return On success, the number of bytes written (excludint the null-termination character).
 * On error, one of httpmessage_result_type
 *
 * @see httpmessage_result_code
 */
HMAPI ssize_t httpmessage_headerfield_write_file(
    FILE *file,
    const httpmessage_headerfield *field);

/**
 * @ingroup header
 * @brief Write a HTTP header line to a buffer.
 *
 * @param output Output buffer
 * @param output_size Output buffer size
 * @param field Header to write
 *
 * @attention NULL termination character may not be written at end of string
 * if the buffer is too small.
 *
 * @return On success, the number of bytes written (excludint the null-termination character).
 * On error, one of httpmessage_result_type
 *
 * @see httpmessage_result_code
 */
HMAPI ssize_t httpmessage_headerfield_write_buffer(
    void *output, size_t output_size,
    const httpmessage_headerfield *field);

/**
 * @ingroup header
 * @brief Parse all headers in the given text
 *
 * Function stops when end of input is reached or if a leading CRLF  is found.
 *
 * @param headerfield_list Header list to populate
 * @param text Input text
 * @param length Input text length
 * @param option_flags Option flags. These flags are passed to httpmessage API functions called internally.
 *
 * @return On success, number of bytes conumed in @c text.
 * On error, one of @c httpmessage_result_code
 *
 * @see httpmessage_result_code
 */
HMAPI int httpmessage_headerfield_list_consume(
    httpmessage_headerfield *headerfield_list,
    const char *text, size_t length,
    int option_flags);

/**
 * @ingroup header
 * @brief Write HTTP headers to a file.
 *
 * @param file Output file
 * @param headerfield_list List of header to write
 *
 * @return On success, the number of bytes written (excludint the null-termination character).
 * On error, one of httpmessage_result_type
 *
 * @see httpmessage_result_code
 */
HMAPI ssize_t httpmessage_headerfield_list_write_file(
    FILE *file,
    const httpmessage_headerfield *headerfield_list);

/**
 * @ingroup header
 * @brief Write a list of HTTP header to a buffer.
 *
 * @param output Output buffer
 * @param output_size Output buffer size
 * @param headerfield_list List of headers to write.
 *
 * @attention NULL termination character may not be written at end of string
 * if the buffer is too small.
 *
 * @return On success, the number of bytes written (excludint the null-termination character).
 * On error, one of httpmessage_result_type
 *
 * @see httpmessage_result_code
 */
HMAPI ssize_t httpmessage_headerfield_list_write_buffer(
    void *output, size_t output_size,
    const httpmessage_headerfield *headerfield_list);



HTTPMESSAGE_C_END

#endif /* LIBHTTPMESSAGGE_HEADER_H__ */

