--------------------------------------------
-- httpmessage module
--------------------------------------------

local moduledir =  path.getdirectory (_SCRIPT)
local workingdir = path.getabsolute(_WORKING_DIR)
local rootdir = path.getabsolute (path.join (moduledir, "../../.."))

local this = {
	rootdir = rootdir,
	scriptdir = path.join (rootdir, "scripts/premake5"),
	version = dofile ("../data/version.lua"),
	location = _OPTIONS["location"] or path.join (rootdir, "build"),
	targetdir = {
		base = _OPTIONS["targetdir"] or path.join (rootdir, "dist"),
		token = "%{cfg.name:gsub('(.*)|(.*)', '%2/%1')}" 
	},
	workspace = function ()
		include (path.join (moduledir, "../workspace.lua"))
	end,
	project = function ()
		include (path.join (moduledir, "../targets/httpmessage.lua"))
	end
}

this.location = iif (
	path.isabsolute (this.location),
	this.location,
	path.join (workingdir, this.location) 
)

this.targetdir.base = iif (
	path.isabsolute (this.targetdir.base),
	this.targetdir.base,
	path.join (workingdir, this.targetdir.base) 
)

function this.use ()
	includedirs {
		path.join (this.rootdir, "include/httpmessage")
	}
	links { "httpmessage" }
end

setmetatable (this.targetdir, {
	__call = function (o, subdir, returnonly)
			local p = path.join (o.base, 
				o.token,
				subdir or ".")
			if not returnonly then
				targetdir (p)
			end
			return p 
	end,
	__targetdir = function (o)
		return o.base
	end
})

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