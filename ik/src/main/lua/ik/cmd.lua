local console = log.open("console")
local sandbox = require "ik.sandbox"
local config  = require "ik.config"

local module = {}

function module.get( path )
    local rootloader = sandbox("ik.loader")
    if not rootloader:load(path) then
        console:error("package.lua file not found :%s",config.current_dir)
    end
end

function module.run(path, ...)
    local rootloader = sandbox("ik.loader")

    rootloader:run(path, ...)
end


return module