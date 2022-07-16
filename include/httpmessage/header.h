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
typedef httpmessage_stringview httpmessage_headerfield;



/**
 * @ingroup header
 *
 * @brief Read the header field name
 *
 * Alias of httpmessage_token_consume()
 *
 * @param field Output field
 * @param field_length Output field length
 * @param text Input text
 * @param length Input text length
 *
 * @return On success, number of bytes of @c text consumed.
 * On error, one of httpmessage_result_code
 *
 * @see httpmessage_result_code
 */
HMAPI int httpmessage_headerfield_consume(
    const char **field,
    size_t *field_length,
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
 * In this case, the @c next_chunk member of @c httpmessage_headervalue
 * will point on another httpmessage_headervalue representing
 * the next chunk of text for the value.
 *
 */
typedef struct __httpmessage_headervalue
{
	/** First chunk of value */
	httpmessage_stringview chunk;
	
	/** Reference the the next header field value chunk */
	struct __httpmessage_headervalue *next_chunk;
} httpmessage_headervalue;

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
HMAPI void httpmessage_headervalue_clear(
    httpmessage_headervalue *value,
    int option_flags);

/**
 * @ingroup header
 *
 * @brief Allocate and initialize a new header value
 *
 * @return The new header value or @c NULL on error
 */
HMAPI httpmessage_headervalue *httpmessage_headervalue_new(void);


/**
 * @ingroup header
 * @brief Initialize a header value
 *
 * @param value Instance to initialize
 */
HMAPI void httpmessage_headervalue_init(
    httpmessage_headervalue *value);

/**
 * @ingroup header
 *
 * @brief Append a new chunk of data to the given header value
 * @param value Header value that will receive the chunk
 * @param text Text to add
 * @param length Text length
 *
 * @return The newly created httpmessage_headervalue
 */
HMAPI httpmessage_headervalue *httpmessage_headervalue_append_chunk(
    httpmessage_headervalue *value,
    const char *text, size_t length);

/**
 * @ingroup header
 *
 * @brief Get the total length of the header value text
 *
 * @param value Header value
 * @return Sum of all header value chunk lenght
 */
HMAPI size_t httpmessage_headervalue_total_length(const httpmessage_headervalue *value);

/**
 * @ingroup header
 *
 * @brief Copy the referenced header value chunks to a contiguous text buffer.
 *
 * @param output Output buffer
 * @param output_size Output buffer size
 * @param value Input header value
 * @return On success, the number of bytes copied to @c output (excluding the NULL termination character).
 * On error, one of the httpmessage_result_code codes
 *
 * @see httpmessage_result_code
 */
HMAPI int httpmessage_headervalue_merge_chunks(
    char *output,
    size_t output_size,
    const httpmessage_headervalue *value);

/**
 * @ingroup header
 *
 * @brief Free header value and following chunks
 *
 * @param value Header value to free
 */
HMAPI void httpmessage_headervalue_free(
    httpmessage_headervalue **value);

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
HMAPI int httpmessage_headervalue_line_consume(
    const char **value, size_t *value_length,
    const char *text, size_t length,
    int option_flags);

/**
 * @ingroup header
 *
 * @brief HTTP header field and value
 *
 */
typedef struct __httpmessage_header
{
	/** Header field name */
	httpmessage_headerfield field;
	/** Header field value */
	httpmessage_headervalue value;
	/** The following header */
	struct __httpmessage_header *next_header;
	
} httpmessage_header;


/**
 * @ingroup header
 *
 * @brief Allocate and initialize a new header object
 *
 * @return The new header object or NULL on error.
 */
HMAPI httpmessage_header *httpmessage_header_new(void);

/**
 * @ingroup header
 *
 * @brief Initialize header members
 *
 * @param header Header structure to initialize
 */
HMAPI void httpmessage_header_init(httpmessage_header *header);

/**
 * @ingroup header
 *
 * @brief Free header memory
 *
 * @param header Header to free
 */
HMAPI void httpmessage_header_free(httpmessage_header **header);

/**
 * @ingroup header
 *
 * @brief Initialize a header descriptor
 *
 * @param header Header descriptor instance
 */
HMAPI void httpmessage_header_init(httpmessage_header *header);


/**
 * @ingroup header
 * @brief Indicates if the given header field name match the expected name.
 *
 * String comparison is case instensitive.
 *
 * @param header Header to test field name
 * @param name Name to match. The string MUST be null-terminated
 *
 * @return Non-zero value if the header field name match @c name
 */
HMAPI int httpmessage_header_is(const httpmessage_header *header,
                                const char *name);

/**
 * @ingroup header
 *
 * @brief Clear header member to their initial state.
 *
 * Except if @c ::HTTPMESSAGE_CLEAR_NO_FREE option is set,
 * all dynamically allocated members are freed.
 *
 * @param header Header to clear
 * @param option_flags Option flags. Supported flags are:
 * - @c ::HTTPMESSAGE_CLEAR_NO_FREE
 *
 * @see httpmessage_option_flags
 */
HMAPI void httpmessage_header_clear(
    httpmessage_header *header,
    int option_flags);

/**
 * @ingroup header
 *
 * @brief Get the number of valid headers in the given header list
 *
 * @param header_list Header list
 * @return Number of element in @c header_list
 */
HMAPI  size_t httpmessage_header_count(httpmessage_header *header_list);

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
 * @param header The new header
 * @param current_header The current header
 * @param text Input text
 * @param length Input text length
 * @param option_flags Option flags. Supported flags are:
 * - @c ::HTTPMESSAGE_CONSUME_IGNORE_MISSING_CRLF
 * - @c ::HTTPMESSAGE_CLEAR_NO_FREE
 *
 * @return On success, the numbre of bytes consumed (the current line length).
 * On error, one of httpmessage_result_code
 *
 *@see httpmessage_result_code
 * @see httpmessage_option_flags
 */
HMAPI int httpmessage_header_line_consume(
    httpmessage_header **header,
    httpmessage_header *current_header,
    const char *text, size_t length,
    int option_flags);

/**
 * @ingroup header
 * @brief Write a HTTP header line to a file
 *
 * @param file Output file
 * @param header Header to write
 *
 * @attention NULL termination character may not be written at end of string
 * if the buffer is too small.
 *
 * @return On success, the number of bytes written (excludint the null-termination character).
 * On error, one of httpmessage_result_type
 *
 * @see httpmessage_result_code
 */
HMAPI ssize_t httpmessage_header_write_file(
    FILE *file,
    const httpmessage_header *header);

/**
 * @ingroup header
 * @brief Write a HTTP header line to a buffer.
 *
 * @param output Output buffer
 * @param output_size Output buffer size
 * @param header Header to write
 *
 * @attention NULL termination character may not be written at end of string
 * if the buffer is too small.
 *
 * @return On success, the number of bytes written (excludint the null-termination character).
 * On error, one of httpmessage_result_type
 *
 * @see httpmessage_result_code
 */
HMAPI ssize_t httpmessage_header_write_buffer(
    void *output, size_t output_size,
    const httpmessage_header *header);

/**
 * @ingroup header
 * @brief Parse all headers in the given text
 *
 * Function stops when end of input is reached or if a leading CRLF  is found.
 *
 * @param header_list Header list to populate
 * @param text Input text
 * @param length Input text length
 * @return On success, number of bytes conumed in @c text.
 * On error, one of @c httpmessage_result_code
 *
 * @see httpmessage_result_code
 */
HMAPI int httpmessage_header_list_consume(
    httpmessage_header *header_list,
    const char *text, size_t length);

/**
 * @ingroup header
 * @brief Write HTTP headers to a file.
 *
 * @param file Output file
 * @param header_list List of header to write
 *
 * @return On success, the number of bytes written (excludint the null-termination character).
 * On error, one of httpmessage_result_type
 *
 * @see httpmessage_result_code
 */
HMAPI ssize_t httpmessage_header_list_write_file(
    FILE *file,
    const httpmessage_header *header_list);

/**
 * @ingroup header
 * @brief Write a list of HTTP header to a buffer.
 *
 * @param output Output buffer
 * @param output_size Output buffer size
 * @param header_list List of headers to write.
 *
 * @attention NULL termination character may not be written at end of string
 * if the buffer is too small.
 *
 * @return On success, the number of bytes written (excludint the null-termination character).
 * On error, one of httpmessage_result_type
 *
 * @see httpmessage_result_code
 */
HMAPI ssize_t httpmessage_header_list_write_buffer(
    void *output, size_t output_size,
    const httpmessage_header *header_list);



HTTPMESSAGE_C_END

#endif /* LIBHTTPMESSAGGE_HEADER_H__ */

