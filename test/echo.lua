--
-- Created by IntelliJ IDEA.
-- User: yayanyang
-- Date: 16/9/10
-- Time: 上午11:50
-- To change this template use File | Settings | File Templates.
--

local ping,pong = ...



while true do
    print(iolua.chan_recv(ping))
    iolua.chan_send(pong,"echo")
end


