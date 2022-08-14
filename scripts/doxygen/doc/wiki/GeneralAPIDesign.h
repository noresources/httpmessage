/**
 * @page GeneralAPIDesign General API design
 * 
 * 
 * ## Function naming
 * 
 * * `<name>_new()` functions allocates and initialize an instance of a struct named `<name>`.
 * * `<name>_storage_new(...)` allocates and initialize an instance of a structure named `<name>` and pre-allocates internal content in a single allocation.
 * * `<name>_init(<name> *instance)` Initialize an instance of `<name>` with the default values. 
 * These functions should be called only once on instance allocated on stack.
 * * `<name>_clear(<name> instance *)` resets instance members to their initial state.
 * * `<name>_free(<name> instance **)` Free an instance memory previously allocated with `<name>_new()` and sets the pointer value to `NULL`
 * * `*_consume` functions read the given input text looking for HTTP message entities and 
 * returns the number of bytes consumed on success or a negative error code on failure.
 * * `*_write_*` functions copy the content of a HTTP message entity to a target and returns
 * the number of bytes written on success or a negative value on error. * 

 */
