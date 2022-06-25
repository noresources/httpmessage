/* text is expected to contain a HTTP message */
int type = httpmessage_message_get_type(text, length, 0);

if (type == HTTPMESSAGE_TYPE_RESPONSE) {
	httpmessage_request request;
	httpmessage_request_init(&request);
	int result = httpmessage_request_consume(
					 &request, text, length,
					 0);
	if (result > 0) {
		/* Success, process ... */
	}
	/* Free internally allocated elements */
	httpmessage_request_request(&request, 0);
} else if (type == HTTPMESSAGE_TYPE_REQUEST) {
	/* Same with httpmessage_response */
;
} else fprintf(stderr, "Invalid message type (%d)\n", type);
