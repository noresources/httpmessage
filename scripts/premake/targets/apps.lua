--------------------------------------------
-- httpmessage tests apps
--------------------------------------------

local module = require "../httpmessage-module"

for _, filepath in pairs (os.matchfiles (path.join (module.rootdir, "src/apps/*.c")))
do
	local name = path.getbasename (filepath)
	project (name)
		location (module.location)
		module.targetdir("bin")
		kind "ConsoleApp"
		language "C"
		warnings "Extra"
		defines {
			"_POSIX_SOURCE" -- fileno()
		}
		includedirs {
			path.join (module.rootdir, "include")
		}
		files {
			filepath
		}
		links {
			"httpmessage"
		}
	end
	
