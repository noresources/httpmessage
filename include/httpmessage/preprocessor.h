/**************************************************************************************$
 * httpmessage
 ***************************************************************************************
 * Copyright © 2022 by Renaud Guillard (dev@nore.fr)
 * Distributed under the terms of the MIT License, see LICENSE
 ***************************************************************************************
 */

/**
 * @file preprocessor.h
 * @brief Preprocessor macro definitions
 */

#if !defined (LIBHTTPMESSAGGE_PREPROCESSOR_H__)
#define LIBHTTPMESSAGGE_PREPROCESSOR_H__

#if !defined (HMAPI)
#	if defined (__GNUC__)
/** @brief C API exposition macro */
#		define HMAPI __attribute__((visibility("default")))
#	else
/** @brief C API exposition macro */
#		define HMAPI
#	endif
#endif

#if defined(__cplusplus)
/**
 * @brief Begin extern C block
 */
#	define HTTPMESSAGE_C_BEGIN 	extern "C" {
/**
 * @brief End exter C block
 */
#	define HTTPMESSAGE_C_END 	}
#else
/**
 * @brief Begin extern C block
 */
#	define HTTPMESSAGE_C_BEGIN
/**
 * @brief End exter C block
 */
#	define HTTPMESSAGE_C_END
#endif

#if !defined(__SIZEOF_POINTER__)
#	error "__SIZEOF_POINTER__ not defined"
#endif

#if (__SIZEOF_POINTER__ == 8)
#	define HTTPMESSAGE_PAD64(variable, size) char variable[size];
#	define HTTPMESSAGE_PAD32(variable, size)
#elif (__SIZEOF_POINTER__ == 4)
#	define HTTPMESSAGE_PAD64(variable, size)
#	define HTTPMESSAGE_PAD32(variable, size) char variable[size];
#else
#	error "Unsupported pointer size"
#endif

#endif /* LIBHTTPMESSAGGE_PREPROCESSOR_H__ */

