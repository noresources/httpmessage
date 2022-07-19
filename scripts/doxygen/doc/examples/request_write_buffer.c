
httpmessage_request request;
httpmessage_request_init(&request);

/*
 * Reminder: stringview does not copy string data.
 */
httpmessage_stringview_assign (&request->method, "GET")
httpmessage_stringview_assign (&request->request_uri, "/foo/bar")

/**
 * Allocate a new header descriptor but do not copy string data
 */
httpmessage_message_append_header(&request->message,
				"Accept", "application/json");

char buffer[1024];
ssize_t written = httpmessage_request_write_buffer(buffer, sizeof (buffer), &request);

/**
 * Free memory allocated by httpmessage_message_append_header
 */
httpmessage_request_clear (&request, 0):

if (written < 0)
{
	fprintf (stderr, "Failed to write request to buffer. Error code: %d\n", written);
	return;
}

fprintf (stdout, "-- Request message ------ \n%.*s", written, buffer);


