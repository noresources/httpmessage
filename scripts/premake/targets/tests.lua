--------------------------------------------
-- httpmessage tests apps
--------------------------------------------

local module = require "../httpmessage-module"

for _, filepath in pairs (os.matchfiles (path.join (module.rootdir, "tests/*.c")))
do
	local name = path.getbasename (filepath)
	project ("test-" .. name)
		location (module.location)
		module.targetdir("tests")
		kind "ConsoleApp"
		language "C"
		includedirs {
		path.join (module.rootdir, "tests"),
			path.join (module.rootdir, "include")
		}
		files {
			filepath
		}
		links {
			"httpmessage"
		}
	end
	
