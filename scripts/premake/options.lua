--------------------------------------------
-- httpmessage premake options
--------------------------------------------

local defaultLocation = path.getabsolute (path.getdirectory(_SCRIPT) .. "/../../build")
local defaultTarget = path.getabsolute (path.getdirectory(_SCRIPT) .. "/../../dist") 

newoption {
	trigger = "location",
	description = "Build script location",
	value = "path",
	default = defaultLocation 
}

newoption {
	trigger = "targetdir",
	description = "Output directory",
	value = "path",
	default = defaultTarget
}

-- premake 4 compatibility

if not (type (_OPTIONS["location"]) == "string")
then 
	_OPTIONS["location"] = defaultLocation
end

if not (type (_OPTIONS["targetdir"]) == "string")
then
	_OPTIONS["targetdir"] = defaultTarget
end

