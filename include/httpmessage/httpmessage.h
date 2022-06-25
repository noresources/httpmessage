/**************************************************************************************$
 * httpmessage
 ***************************************************************************************
 * Copyright © 2022 by Renaud Guillard (dev@nore.fr)
 * Distributed under the terms of the MIT License, see LICENSE
 ***************************************************************************************
 */

/**
 * @file httpmessage.h
 * @brief Library main header
 */

#if !defined (LIBHTTPMESSAGGE_H__)
#define LIBHTTPMESSAGGE_H__

#include "httpmessage/message.h"
#include "httpmessage/header.h"
#include "httpmessage/text.h"
#include "httpmessage/grammar.h"
#include "httpmessage/preprocessor.h"

HTTPMESSAGE_C_BEGIN

/**
 * @ingroup base
 *
 * httpmessage function result codes
 */
typedef enum __httpmessage_result_code
{
	HTTPMESSAGE_OK = 0,                     /**< Successful operation */
	HTTPMESSAGE_ERROR_INVALID_ARGUMENT = -1,/**< One or more function arguments are invalid */
	HTTPMESSAGE_ERROR_OVERFLOW = -2,        /**< Output buffer will overflow */
	HTTPMESSAGE_ERROR_SYNTAX = -1000,        /**< The text given in argument does not match the expected syntax */
	HTTPMESSAGE_ERROR_INCOMPLETE = -1001, 	/**< */
} httpmessage_result_code;

/**
 * @ingroup base
 *
 * @ingroup message
 * @brief HTTP message type
 * @see https://datatracker.ietf.org/doc/html/rfc2616#section-4
 */
typedef enum
{
	HTTPMESSAGE_TYPE_UNKNOWN = -1, /**< Unknown message type */
	HTTPMESSAGE_TYPE_REQUEST = 'q',/**< HTTP request message */
	HTTPMESSAGE_TYPE_RESPONSE = 'r'/**< HTTP response message */
} httpmessage_message_type;

/**
 * @ingroup base
 *
 * @brief Option flags
 */
typedef enum
{
	/**
	 * @brief Accept text input where an expected trailing CRLF is missing.
	 *
	 * Applies to all @c *_consume functions.
	 */
	HTTPMESSAGE_CONSUME_IGNORE_MISSING_CRLF = (1 << 0),
	
	/**
	 * Instead of freeing structure members,
	 * set them to a value that mark them as unused.
	 *
	 * Ex. Set header value subchunks length to zero.
	 */
	HTTPMESSAGE_CLEAR_NO_FREE = (1 << 1)
} httpmessage_option_flags;

HTTPMESSAGE_C_END

#endif /* LIBHTTPMESSAGGE_H__ */
