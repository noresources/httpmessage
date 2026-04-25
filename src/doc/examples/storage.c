/*
 * Accepts request with at most 8 header fields
 */
#define MAX_HEADER_FIELD 8

/*
 * Accepts header field value on at most 2 lines
 *
 * Such as
 *
 * MetaHeader: foo
 *  bar
 */
#define MAX_VALUE_LINE 2

httpmessage_request *request = httpmessage_request_storage_new(
			              MAX_HEADER_FIELD,
			              MAX_VALUE_LINE);

/*
 * These option flags are MANDATORY to
 * tell consume functions to NOT allocate nor free
 * any memory.
 */
int mandatory_option_flags = (
				HTTPMESSAGE_CLEAR_NO_FREE
				| HTTPMESSAGE_NO_ALLOCATION
				);

while ( some_condition )
{
	/* Read client request */

	/*
	 * Consume request data contained text
	 *
	 * If request contains more than MAX_HEADER_FIELD header fields
	 * or have header field value on more than 2 linkes,
	 * this function will return HTTPMESSAGE_ERROR_ALLOCATION
	 */
	int result = httpmessage_request_consume(request,
					text, text_length,
					mandatory_option_flags);

	if (result > 0)
	{
		/* Process content of request as usual */
	}
}

/* Free request memory block */
free (request);









