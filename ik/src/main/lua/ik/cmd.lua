local console = log.open("console")
local sandbox = require "ik.sandbox"
local config  = require "ik.config"

local module = {}

function module.get( ... )
    local rootloader = sandbox("ik.loader")
    if not rootloader:load(config.current_dir) then
        console:error("package.lua file not found :%s",config.current_dir)
    end
end

function module.run(...)
    local rootloader = sandbox("ik.loader")
    rootloader:run(...)
end

return module