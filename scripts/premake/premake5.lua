--------------------------------------------
-- httpmessage build environment
--------------------------------------------

-- Modules
if not (type(requireopt) == "function") then
	requireopt = function () premake.warn("This version of premake does not have requireopt function")
		return false, "requireopt not supported"
	end
end 
if not (requireopt "doxygen")
then
	premake.warn ("doxygen module not available")
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
	