--------------------------------------------
-- httpmessage build environment
--------------------------------------------

-- Modules
if _OPTIONS["require-doxygen"]
then
	require "doxygen"
else
	doxygen = function () end
	doxyfile = function () end
end

local module = require "httpmessage-module"

-- Options
include "options.lua"

-- Actions
include "actions/preprocess.lua"

-- Workspace
	module.workspace ()
	