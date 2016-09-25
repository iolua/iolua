local n = ...
local c = chan.create()
task.create("chan_test_echo.lua",c,n)
for v = 1,n,1 do
    chan.recv(c)
    chan.send(c,"hello",v, "task")
end

chan.close(c)
