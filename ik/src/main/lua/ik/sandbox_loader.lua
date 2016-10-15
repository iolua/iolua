local console = log.open("console")
local ops = require "ik.ops"


local writer,reader,modulename = ...
local module = {}

local module = require(modulename)

local obj = module.ctor(table.unpack(table.pack(...),4))

while true do

    local args = table.pack(pcall(chan.recv,reader))

    if not args[1] then -- the sandbox was closed
        return 
    end

    local method = module[args[2]]

    if method == nil then
        chan.send(writer, false, string.format("unknown method :%s", args[2]))
    else
        
        local retargs = table.pack(pcall(method,obj,table.unpack(args,3)))

        if retargs[1] then
            chan.send(writer, true, table.unpack(retargs,2))
        else
            chan.send(writer, false, retargs[2])
        end
    end
end
