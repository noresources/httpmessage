/**
 * @page MemoryManagement Memory management
 * 
 * 
 * By default, httpmessage library  does not allocates any memory to
 * store text chunks. It only creates descriptor structures that point
 * to text memory block allocated elsewhere.
 * 
 * # Allocation and deallocation functions usage
 * 
 * ## Stack allocation
 * 
 * ```
 * /* Declare and initialize members */
 * struct httpmessage_request descriptor;
 * httpmessage_request_init(&descriptor);
 * 
 * /* ... processing ... */
 * 
 * /* Free memory allocated durring processing */
 * httpmessage_request_clear (&descriptor, 0);
 * ```
 * 
 * ## Heap allocation with dynamic internal storage
 * 
 * ```
 * struct httpmessage_request *descriptor = httpmessage_request_new();
 * 
 * /* ... processing ... */
 * 
 * httpmessage_request_free (descriptor);
 * ```
 * 
 * ## Heap allocation with pre-allocated storage
 * 
 * ```
 * struct httpmessage_request *storage = httpmessage_request_storage_new(8, 2);
 * int option_flags = HTTPMESSAGE_CLEAR_NO_FREE | HTTPMESSAGE_NO_ALLOCATION;
 * 
 * /* ... processing ... 
 * httpmessage_request_consume(descriptor, ..., option_flags);
 * */
 * free (descriptor);
 * ```
 * 
 * 
 * 
 * 
 *  * 

 */
