local n = ...
local c = chan.create()
for v = 1,n,1 do
    task.create("chan_test_echo.lua",c,"hello",v, "task")
    chan.recv(c)
end

chan.close(c)