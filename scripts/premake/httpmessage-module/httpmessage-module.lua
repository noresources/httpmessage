--------------------------------------------
-- httpmessage module
--------------------------------------------

local moduledir =  path.getdirectory (_SCRIPT)
local rootdir = path.getabsolute (path.join (moduledir, "../../.."))

local this = {
	rootdir = rootdir,
	scriptdir = path.join (rootdir, "scripts/premake5"),
	version = dofile ("../data/version.lua"),
	location = _OPTIONS["location"] or path.join (rootdir, "build"),
	targetdir = _OPTIONS["targetdir"] or path.join (rootdir, "dist"),
	workspace = function ()
		include (path.join (moduledir, "../workspace.lua"))
	end,
	project = function ()
		include (path.join (moduledir, "../targets/httpmessage.lua"))
	end
}

function this.use ()
	includedirs {
		path.join (this.rootdir, "include/httpmessage")
	}
	links { "httpmessage" }
end

setmetatable (this.version, {
	__index = function (o, k)
			if k == "string" then
				return o.major .. "." .. o.minor .. "." .. o.patch
			elseif k == "number" then
				return (o.major * 10000) + (o.minor * 100) + o.patch
			end
	end
})

setmetatable (this, {
	__index = function (o, k)
		if k == "_VERSION" then
			return this.version.string
		end
	end
})

return this