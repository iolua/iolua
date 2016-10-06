local console = log.open("console")
local sandbox = require "ik.sandbox"
local config  = require "ik.config"

local module = {}

function module.get( ... )
    local ok,data = sandbox.create(config.homepath .. "/src/main/lua/ik/loader.lua",fs.current_path())

    if not ok then
        console:error(data)
    end
end

return module