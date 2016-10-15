local config    = require "ik.config"
local ops       = require "ik.ops"
local console   = log.open("console")
local module    = {}

function module.ctor()
    return {}
end

function module:load(...)
    print(self,...)
end

return module