--------------------------------------------
-- httpmessage default workspace
--------------------------------------------

local module = require "httpmessage-module"

local wks = workspace "httpmessage"
do
	configurations { "Debug", "Release" }
	location (module.location)
	targetdir (module.targetdir.base)
	objdir (path.join (module.targetdir.base, "obj"))
	
	filter "configurations:Debug"
		runtime 'Debug'
		inlining "Disabled"
		symbols 'On'
		optimize 'Off'
	filter "configurations:Release"
		runtime 'Release'
		inlining 'Auto'
		symbols 'Off'
		optimize 'Speed'
	filter {}
	
	filter { "system:" .. premake.MACOSX }
		systemversion "10.11"
	filter {}
	
	if type(doxyfile) == "function" then
		doxyfile {
				project_name = "httpmessage",
				project_brief = "HTTP message parsing library",
				project_number = module.version.string,
				ignore_prefix = {
					"httpmessage_"
				},
				input = table.flatten ({
					os.matchfiles (path.join (module.location, "doc/wiki/*.h")), 
					os.matchfiles (path.join (module.location, "doc/*.h")),
				}),
				typedef_hides_struct = true,
				example_path = path.join (module.location, "doc/examples")
		}
	end

-- Projects
	for _, f in ipairs (os.matchfiles("targets/**.lua")) do
		include (f)
	end	
end

return wks