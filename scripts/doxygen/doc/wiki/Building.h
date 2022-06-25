/**
 * @page Building Building httpmessage
 * 
 * 
 * ## Using pre-generated build scripts
 * 
 * Makefile and IDE projects are available in the `scripts` folder.
 * 
 * ```
 * make -C scripts/linux/gmake2
 * ```
 * 
 * ## Using premake scripts 
 * 
 * `httpmessage` provides a premake module.
 * 
 * ```
 * local httpmessage = require "{httpmessage-src-path}"
 * -- NB: httpmessage-src-path must ends by a "httpmessage" folder
 * 
 * workspace "MyHTTPBasedProject"
 * 	configurations { "Debug", "Release" }
 * 	
 * 	httpmessage.project () -- Declare httpmessage library project
 * 	
 * 	project "HTTPApp"
 * 		kind "ConsoleApp"
 * 		httpmessage.use () -- Adds required includedirs and links instructions
 * ```
 *  * 

 */
