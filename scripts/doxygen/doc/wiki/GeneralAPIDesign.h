/**
 * @page GeneralAPIDesign General API design
 * 
 * 
 * ## Function naming
 * 
 * * `<name>_new()` functions allocates and initialize an instance of a struct named `<name>`.
 * * `<name>_init(<name> *instance)` Initialize an instance of `<name>` with the default values. 
 * The should be called only once on instance allocated on stack.
 * * `<name>_clear(<name> instance *)` resets instance members to their initial state.
 * * `<name>_free(<name> instance **)` Free an instance memory previously allocated with `<name>_new()` and sets the pointer value to `NULL`
 * * `*_consume` functions read the given input text looking for HTTP message entities and 
 * returns the number of bytes consumed on success or a negative error code on failure. * 

 */
