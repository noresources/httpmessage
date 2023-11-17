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

#if !defined (HTTPMESSAGE_POINTER_SIZE)
#	if defined(__SIZEOF_POINTER__)
#		define HTTPMESSAGE_POINTER_SIZE (__SIZEOF_POINTER__)
#	elif (defined(_WIN64))
|| (defined(__arm64__)&& (__arm64__))
|| (defined(_LP64)&& _LP64))
|| (defined(__x86_64)&& (__x86_64))
|| (defined(__x86_64__)&& (__x86_64__))
#		define HTTPMESSAGE_POINTER_SIZE 8
#	elif defined (__ARMEL__) && (__ARMEL__)
#		if defined(__ARM_ARCH_ISA_A64) && __ARM_ARCH_ISA_A64
#			define HTTPMESSAGE_POINTER_SIZE 8
#		else
#			define HTTPMESSAGE_POINTER_SIZE 4
#		endif
#	elif (defined (__i386) && (__i386)) \
		|| (defined(__i386__)&& (__i386__)) \
		|| (defined(__i486)&& (__i486)) \
		|| (defined(__i486__)&& (__i486__)) \
		|| (defined(__i686)&& (__i686)) \
		|| (defined(__i686__)&& (__i686__)) \
		|| defined(_WIN32)
#		define HTTPMESSAGE_POINTER_SIZE 4
#	else
#		error "Unable to detect pointer size"
#	endif
#endif

#if (HTTPMESSAGE_POINTER_SIZE == 8)
#	define HTTPMESSAGE_PAD64(variable, size) char variable[size];
#	define HTTPMESSAGE_PAD32(variable, size)
#elif (HTTPMESSAGE_POINTER_SIZE == 4)
#	define HTTPMESSAGE_PAD64(variable, size)
#	define HTTPMESSAGE_PAD32(variable, size) char variable[size];
#else
#	error "Unsupported pointer size"
#endif

/**
 * @ingroup text
 * @brief Write content to file
 */
#define HTTPMESSAGE_TEXT_WRITE_FILE(_written, _file, _text, _length) { \
	size_t _result = fwrite(_text, (size_t)1, (size_t)_length, _file); \
	if ((size_t)_result != (size_t)(_length)) return HTTPMESSAGE_ERROR_WRITE; \
	_written += (size_t)_length; \
}

/**
 * @brief Write content to a file using *printf functions
 */
#define HTTPMESSAGE_PRINTF_FILE(_written, _file, ...) { \
	int _result = fprintf (_file, __VA_ARGS__); \
	if (_result < 0) return _result; \
	_written += (size_t)_result; \
}

/**
 * @brief Write string view content to file.
 */
#define HTTPMESSAGE_STRING_WRITE_FILE(_written, _file, _string) \
		HTTPMESSAGE_TEXT_WRITE_FILE(_written, _file, (_string).text, (_string).length)

/**
 * @brief Write arbitrary text to a buffer.
 * update I/O buffer pointer and length
 */
#define HTTPMESSAGE_TEXT_WRITE_BUFFER(_output, _output_size, _text, _length) { \
	if ((size_t)(_output_size) < (size_t)(_length)) return HTTPMESSAGE_ERROR_OVERFLOW; \
	memcpy (_output, _text, _length); \
	_output += (size_t)(_length); _output_size -= (size_t)(_length); \
}

/**
 * @brief Format a text and write it to a buffer.
 * update I/O buffer pointer and length
 */
#define HTTPMESSAGE_PRINTF_BUFFER(_output, _output_size, ...) { \
	int _result = snprintf (_output, _output_size, __VA_ARGS__); \
	if (_result < 0) return _result; \
	_output += (size_t)_result; _output_size -= (size_t)_result; \
}

/**
 * Copy a string view to a buffer.
 */
#define HTTPMESSAGE_STRING_WRITE_BUFFER(_output, _output_size, _string) \
		HTTPMESSAGE_TEXT_WRITE_BUFFER(_output, _output_size, (_string).text, (_string).length)

#endif /* LIBHTTPMESSAGGE_PREPROCESSOR_H__ */

