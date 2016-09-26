local n = ...
local c = chan.create()

local logger = log.open("console")

for v = 1,n,1 do
    task.create("chan_test_echo.lua",c,"hello",v, "task")
    chan.recv(c)

--    logger:debug("%d",v)
end

chan.close(c)