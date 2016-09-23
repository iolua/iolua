--
-- Created by IntelliJ IDEA.
-- User: yayanyang
-- Date: 16/9/9
-- Time: 下午3:53
-- To change this template use File | Settings | File Templates.
--

local logger = log.open("console")

for v = 1,10000,1 do
    logger:debug("create task(%d)",task.create("helloworld.lua","hello",v))
end





