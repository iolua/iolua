local console = log.open("console")
local sandbox = require "ik.sandbox"
local config  = require "ik.config"

local module = {}

function module.get( ... )
    local loader = sandbox("ik.loader")
    loader:load("test",1,2,3)
end

return module