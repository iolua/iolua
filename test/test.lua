--
-- Created by IntelliJ IDEA.
-- User: yayanyang
-- Date: 16/9/9
-- Time: 下午3:53
-- To change this template use File | Settings | File Templates.
--

iolua.task("helloworld.lua",1,2)

iolua.task("helloworld.lua",1,"test")

iolua.task("helloworld.lua",1,"test")

iolua.task("helloworld.lua",1,2,3,4,5)

local ping = iolua.chan()
local pong = iolua.chan()

iolua.task("echo.lua",ping,pong)

while true do
    iolua.chan_send(ping,"hello world")
    print(iolua.chan_recv(pong))
end




