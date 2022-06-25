--------------------------------------------
-- httpmessage library - Create / Update generated files
--------------------------------------------

local module = require "../httpmessage-module"

function updateversion ()
	local filename = path.join (
		module.rootdir,
		"include/httpmessage/version.h"
	) 
	local content = io.readfile (filename)
	content = content:gsub ("\r\n", "\n")
	content = content:gsub ("\r", "\n")
	
	for _, k in ipairs({
		"major", "minor", "patch", "number", "string"
	}) do
		local v = module.version[k]
		if type (v) == "string" then
			v = '"' .. premake.esc(v) .. '"'
		end
		local pattern = "(#define%s+HTTPMESSAGE_VERSION_" .. k:upper() .. ").-\n"
		local replacement = "%1 " .. v .. "\n"
		content = content:gsub(pattern, replacement)
	end
	
	io.writefile(filename, content)
end

function markdowntodoxygen ()
	for _, md in ipairs (os.matchfiles(path.join(module.rootdir, "wiki", "*.md")))
	do
		local locationdir = path.join (
				_OPTIONS["location"] or module.rootdir,
				"doc"
		)
		local content = io.readfile(md)
		
		-- Translate internal links
		content = content:gsub("%[(.-)%]%(([a-zA-Z0-9_-]+)%)", "@ref %2")
		-- Other links
		content = content:gsub("%[(.-)%]%(([a-zA-Z0-9_-]+)%)", "%2")
		
		local name  = path.getbasename (md)
		local tag = iif(name == "Home", "mainpage", "page")
		local title = content:gsub("(.-)\n.*", "%1")
		local header = path.join (locationdir, name .. ".h")
		
		-- Remove main title
		content = content:gsub (".-\n====-\n(.*)", "%1")
		
		io.writefile (header, 
			"/**\n * @" .. tag .. " " .. name .. " " .. title .. "\n"
					.. " * \n"
					.. " * "
					.. content:gsub("\n", "\n * ")
					.. " * \n"
					.. "\n */\n")
	end
end

newaction ({
	trigger = "preprocess",
	shortname = "Generate files",
	description = "Create or update generated files",
	execute  = function ()
			updateversion()
			markdowntodoxygen()
	end
})
