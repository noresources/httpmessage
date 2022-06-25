# httpmessage - Lightweight HTTP message parsing library

This library aims to provide HTTP message parsing
with low memory footprint and no external library dependencies.

Parsing functions only allocate descriptors that locates
HTTP grammar elemets position and length in a HTTP message
text buffer.

## Features
* Request line parsing
  * Method
  * Request URI
  * HTTP version
* Status line parsing
  * HTTP version
  * Status code
  * Reason phrase
* Header line parsing
  * Header field name
  * Header value (multiline support)

## Documentation

See the [API documentation](http://httpmessage.nore.fr/)

## References
* [RFC 2616](https://datatracker.ietf.org/doc/html/rfc2616)

