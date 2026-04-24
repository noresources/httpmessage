--------------------------------------------
-- httpmessage build environment
--------------------------------------------

-- Options
include "options.lua"


-- Modules
if _OPTIONS["require-doxygen"]
then
	require "doxygen"
else
	doxygen = function () end
	doxyfile = function () end
end

local module = require "httpmessage-module"


-- Actions
include "actions/preprocess.lua"

-- Workspace
	module.workspace ()
	