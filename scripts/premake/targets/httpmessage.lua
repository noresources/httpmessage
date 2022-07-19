--------------------------------------------
-- httpmessage library
--------------------------------------------

local module = require "../httpmessage-module"

local prj = project "httpmessage"
	location (module.location)
	module.targetdir("lib")
	kind "StaticLib"
	language "C"
	warnings "Extra"
	includedirs {
		path.join (module.rootdir, "include")
	}
	files {
		path.join (module.rootdir, "include/httpmessage/*.h"),
		path.join (module.rootdir, "src/httpmessage/*.c")
	}

return prj	