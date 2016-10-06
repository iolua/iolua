local config    = require "ik.config"
local module    = {}

function module.create( name, ... )
 
    local c = chan.create()
    
    task.create(config.homepath .. "/src/main/lua/ik/sandbox_init.lua",c,name,...)

    local ret = table.pack(chan.recv(c))

    chan.close(c)

    return table.unpack(ret,2)
end

return module