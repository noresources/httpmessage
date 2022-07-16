/**************************************************************************************$
 * httpmessage
 ***************************************************************************************
 * Copyright © 2022 by Renaud Guillard (dev@nore.fr)
 * Distributed under the terms of the MIT License, see LICENSE
 ***************************************************************************************
 */

/**
 * @file message.h
 * @brief HTTP request and response
 */

#if !defined (LIBHTTPMESSAGGE_MESSAGE_H__)
#define LIBHTTPMESSAGGE_MESSAGE_H__

#include "httpmessage/header.h"
#include "httpmessage/preprocessor.h"

HTTPMESSAGE_C_BEGIN

/**
 * @ingroup message
 *
 * @brief HTTP message body
 */
typedef httpmessage_stringview httpmessage_body;

/**
 * @ingroup message
 *
 * @brief HTTP generic message
 *
 * @see https://datatracker.ietf.org/doc/html/rfc2616#section-4
 */
typedef struct __httpmessage_message
{
	/**
	 * @brief Message type
	 *
	 * @see httpmessage_message_type
	 */
	int type;
	/** HTTP protocol major version */
	int major_version;
	/** HTTP protocol minor version */
	int minor_version;
	
	HTTPMESSAGE_PAD64(__padding, 4) /**< structure padding */
	
	/** Header list */
	httpmessage_header header_list;
	/** Message body */
	httpmessage_body 	body;
} httpmessage_message;

/**
 * @ingroup message
 *
 * @brief Initialize a HTTP message descriptor.
 *
 * @param message Message descriptor instance
 */
HMAPI void httpmessage_message_init(httpmessage_message *message);

/**
 * @ingroup message
 *
 * @brief Clear message
 *
 * @param message Message instance
 * @param option_flags Option flags. Supported flags are
 * - ::HTTPMESSAGE_CLEAR_NO_FREE
 *
 * @see httpmessage_option_flags
 */
HMAPI void httpmessage_message_clear(
    httpmessage_message *message,
    int option_flags);

/**
 * @ingroup message
 * @brief Append a header to the given message.
 *
 * @param message Message to add the header to.
 * @param name Header field name
 * @param value Header value
 * @param option_flags Option flags. Supported flags are
 * - ::HTTPMESSAGE_CLEAR_NO_FREE
 *
 * @return HTTPMESSAGE_OK on success. One of httpmessage_result_code error codes on error.
 */
HMAPI int httpmessage_message_append_header(
    httpmessage_message *message,
    const char *name, const char *value,
    int option_flags);

/**
 * @ingroup message
 *
 * @brief HTTP request message
 *
 * @extends httpmessage_message
 *
 * @see https://datatracker.ietf.org/doc/html/rfc2616#section-5
 */
typedef struct __httpmessage_request
{
	/** HTTP message common data */
	httpmessage_message message;
	
	/** HTTP method */
	httpmessage_stringview method;
	
	/** Request request_uri path */
	httpmessage_stringview request_uri;
	
} httpmessage_request;

/**
 * @ingroup message
 *
 * @brief Initialize a HTTP request descriptor
 *
 * @param request Request descriptor instance
 */
HMAPI void httpmessage_request_init(httpmessage_request *request);

/**
 * @ingroup message
 *
 * @brief Clear request message content
 *
 * @param request Request to clear.
  * @param option_flags Option flags. Supported flags are
 * - ::HTTPMESSAGE_CLEAR_NO_FREE
 *
 * @see httpmessage_option_flags
 */
HMAPI void httpmessage_request_clear(
    httpmessage_request *request,
    int option_flags);

/**
 * @ingroup message
 *
 * @brief HTTP response message
 *
 * @extends httpmessage_message
 *
 * @see https://datatracker.ietf.org/doc/html/rfc2616#section-6
 */
typedef struct __httpmessage_response
{
	/** Message common data */
	httpmessage_message message;
	
	/** HTTP response status */
	int status_code;
	
	HTTPMESSAGE_PAD64(__padding, 4) /**< structure padding */
	
	/** HTTP response status text */
	httpmessage_stringview reason_phrase;
} httpmessage_response;

/**
 * @ingroup message
 *
 * @brief Initialize a HTTP response descriptor.
 *
 * @param response Response descriptor instance
 */
HMAPI void httpmessage_response_init(httpmessage_response *response);

/**
 * @ingroup message
 *
 * @brief Clear response message.
 *
 * @param response Response to clear.
  * @param option_flags Option flags. Supported flags are
 * - ::HTTPMESSAGE_CLEAR_NO_FREE
 *
 * @see httpmessage_option_flags
 */
HMAPI void httpmessage_response_clear(
    httpmessage_response *response,
    int option_flags
);

/**
 * @ingroup message
 *
 * @brief Read HTTP version token
 *
 * ```
 * HTTP-Version   = "HTTP" "/" 1*DIGIT "." 1*DIGIT
 * ```
 *
 * @see https://datatracker.ietf.org/doc/html/rfc2616#section-3.1
 *
 * @param major_version Output major version
 * @param minor_version Output minor version
 * @param text Input text
 * @param length Input text lengh
 * @return On success, number of bytes consumed in @c text.
 * On error, one of httpmessage_result_code
 *
 * @see httpmessage_result_code
 */
HMAPI int httpmessage_message_http_version_consume(
    int *major_version,
    int *minor_version,
    const char *text, size_t length);

/**
 * @ingroup message
 *
 * @brief Read hte Request-URI resourcce of a request line
 *
 * ```
 * Request-URI    = "*" | absoluteURI | abs_path | authority
 * ```
 *
 * @see https://datatracker.ietf.org/doc/html/rfc2616#section-5.1.2
 *
 * @param request_uri Output string view
 * @param text Input text
 * @param length Input text length
 * @return On success, number of bytes consumed in @c text. On error, on of httpmessage_result_code.
 *
 * @see https://datatracker.ietf.org/doc/html/rfc2616#section-5.1.2
 * @todo Better URI parsing
 *
 * @see httpmessage_result_code
 */
HMAPI int httpmessage_request_request_uri_consume(
    httpmessage_stringview *request_uri,
    const char *text, size_t length);

/**
 * @ingroup message
 * @brief Read the first line of a HTTP request message.
 *
 * ```
 * Request-Line   = Method SP Request-URI SP HTTP-Version CRLF
 * ```
 *
 * @see https://datatracker.ietf.org/doc/html/rfc2616#section-5.1
 *
 * @param method Output request method
 * @param request_uri Output request URI
 * @param major_version Output HTTP majour version
 * @param minor_version Output HTTP minor version
 * @param text Input text
 * @param length Input text length
 * @param option_flags Option flags. Supported flags are
 * - ::HTTPMESSAGE_CONSUME_IGNORE_MISSING_CRLF
 *
 * @return On success, number of bytes consumed in @c text.
 * On error, on of httpmessage_reult_code.
 *
 * @see httpmessage_result_code
 * @see httpmessage_option_flags
 *
 */
HMAPI int httpmessage_request_line_consume(
    httpmessage_stringview *method,
    httpmessage_stringview *request_uri,
    int *major_version,
    int *minor_version,
    const char *text, size_t length,
    int option_flags);

/**
 * @ingroup message
 *
 * @brief Read HTTM message status line
 *
 * ```
 * Status-Line = HTTP-Version SP Status-Code SP Reason-Phrase CRLF
 * ```
 *
 * @see https://datatracker.ietf.org/doc/html/rfc2616#section-6.1
 *
 * @param major_version Output major version
 * @param minor_version Output minor version
 * @param status_code Output status code
 * @param reason_phrase Output response phrase
 * @param text Input text
 * @param length Input text length
 * @param option_flags Option flags. Supported flags are
 * - ::HTTPMESSAGE_CONSUME_IGNORE_MISSING_CRLF
 *
 * @return On success, number of bytes consumed in @c text. On error one of httpmessage_result_code
 *
 * @see httpmessage_result_code
 * @see httpmessage_option_flags
 */
HMAPI int httpmessage_status_line_consume(
    int *major_version,
    int *minor_version,
    int *status_code,
    httpmessage_stringview *reason_phrase,
    const char *text,
    size_t length,
    int option_flags);

/**
 * @ingroup message
 *
 * @brief Guess HTTP message type by parsing the first line of the message.
 *
 * @param text Input text
 * @param length Input length
 * @param option_flags Option flags. Supported flags are
 * - ::HTTPMESSAGE_CONSUME_IGNORE_MISSING_CRLF
 *
 * @return One of httpmessage_message_type values
 *
 * @see httpmessage_message_type
 * @see httpmessage_option_flags
 *
 *
 */
HMAPI int httpmessage_message_get_type(
    const char *text, size_t length,
    int option_flags);

/**
 * @ingroup message
 * @brief Read HTTP message headers and body.
 *
 * @param message Output message descriptor
 * @param text Input text
 * @param length Input text length
* @param option_flags Option flags. Supported flags are
 * - ::HTTPMESSAGE_CONSUME_IGNORE_MISSING_CRLF
 *
 * @return On success, number of bytes consumed in @c text.
 * On error, one of httpmessage_result_code
 *
 * @see httpmessage_result_code
 * @see httpmessage_option_flags
 */
HMAPI int httpmessage_message_content_consume(
    httpmessage_message *message,
    const char *text, size_t length,
    int option_flags);

/**
 * @ingroup message
 *
 * @brief Write message header and body to a file
 *
 * @param file Output file descriptor
 * @param message HTTP message content to write
 *
 * @return On success, the number of bytes written.
 * On error, one of httpmessage_result_type
 *
 * @see httpmessage_result_code
 */
HMAPI ssize_t httpmessage_message_content_write_file(
    FILE *file,
    const httpmessage_message *message);

/**
 * @ingroup message
 *
 * @brief Write message header and body to a buffer
 *
 * @param output Output buffer
 * @param output_size Output buffer size
 * @param message Message to write
 *
 * @attention NULL termination character may not be written at end of string
 * if the buffer is too small.
 *
 * @return On success, the number of bytes written (excludint the null-termination character).
 * On error, one of httpmessage_result_type
 *
 * @see httpmessage_result_code
 */
HMAPI ssize_t httpmessage_message_content_write_buffer(
    void *output, size_t output_size,
    const httpmessage_message *message);

/**
 *  @ingroup message
 *  @brief Read a HTTP request message.
 *
 * @param request Output request
 * @param text Input text
 * @param length Input text length
 * @param option_flags Option flags. Supported flags are
 * - ::HTTPMESSAGE_CONSUME_IGNORE_MISSING_CRLF
 *
 * @return On success, number of bytes consumed in @c text.
 * On error, one of httpmessage_result_code
 *
 * @see httpmessage_result_code
 * @see httpmessage_option_flags
 */
HMAPI int httpmessage_request_consume(
    httpmessage_request *request,
    const char *text, size_t length,
    int option_flags);

/**
 * @ingroup message
 *
 * @brief Write a HTTP request message to a file.
 *
 * @param file Output file
 * @param request Request to write
 *
 * @return On success, the number of bytes written (excludint the null-termination character).
 * On error, one of httpmessage_result_type
 *
 * @see httpmessage_result_code
 */
HMAPI ssize_t httpmessage_request_write_file(
    FILE *file,
    const httpmessage_request *request);

/**
 * @ingroup message
 * @brief Write a HTTP request message to a buffer.
 *
 * @param output Output buffer
 * @param output_size Output buffer size
 * @param request Request to write
 * @attention NULL termination character may not be written at end of string
 * if the buffer is too small.
 *
 * @return On success, the number of bytes written (excludint the null-termination character).
 * On error, one of httpmessage_result_type
 *
 * @see httpmessage_result_code
 */
HMAPI ssize_t httpmessage_request_write_buffer(
    void *output, size_t output_size,
    const httpmessage_request *request);


/**
 *  @ingroup message
 *  @brief Read a HTTP response message.
 *
 * @param response Output request
 * @param text Input text
 * @param length Input text length
 * @param option_flags Option flags. Supported flags are
 * - ::HTTPMESSAGE_CONSUME_IGNORE_MISSING_CRLF
 *
 * @return On success, number of bytes consumed in @c text.
 * On error, one of httpmessage_result_code
 *
 * @see httpmessage_result_code
 * @see httpmessage_option_flags
 */
HMAPI int httpmessage_response_consume(
    httpmessage_response *response,
    const char *text, size_t length,
    int option_flags);

/**
 * @ingroup message
 * @brief Write a HTTP response to a file.
 *
 * @param file Output file.
 * @param response Response to write
 *
 * @return On success, the number of bytes written (excludint the null-termination character).
 * On error, one of httpmessage_result_type
 *
 * @see httpmessage_result_code
 */
HMAPI ssize_t httpmessage_response_write_file(
    FILE *file,
    const httpmessage_response *response);

/**
 * @ingroup message
 * @brief Write a HTTP response to a buffer.
 *
 * @param output Output buffer
 * @param output_size Output buffer size
 * @param response Response to write
 *
 * @attention NULL termination character may not be written at end of string
 * if the buffer is too small.
 *
 * @return On success, the number of bytes written (excludint the null-termination character).
 * On error, one of httpmessage_result_type
 *
 * @see httpmessage_result_code
 */
HMAPI ssize_t httpmessage_response_write_buffer(
    void *output, size_t output_size,
    const httpmessage_response *response);

HTTPMESSAGE_C_END

#endif /* LIBHTTPMESSAGGE_MESSAGE_H__ */

