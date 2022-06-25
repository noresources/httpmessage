--------------------------------------------
-- httpmessage default workspace
--------------------------------------------

local module = require "httpmessage-module"

local wks = workspace "httpmessage"
do
	configurations { "Debug", "Release" }
	location (module.location)
	targetdir (module.targetdir)
	
	if type(doxyfile) == "function" then
		doxyfile {
				project_name = "httpmessage",
				project_brief = "HTTP message parsing library",
				project_number = module.version.string,
				ignore_prefix = {
					"httpmessage_"
				},
				input = table.flatten ({
					os.matchfiles (path.join (path.getabsolute(module.location), "doc/*.h")), 
					os.matchfiles (path.join (module.rootdir, "src/doc/*.h"))
				}),
				typedef_hides_struct = true
		}
	end

-- Projects
	for _, f in ipairs (os.matchfiles("targets/**.lua")) do
		include (f)
	end	
end

return wks