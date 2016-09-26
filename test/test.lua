--
-- Created by IntelliJ IDEA.
-- User: yayanyang
-- Date: 16/9/9
-- Time: 下午3:53
-- To change this template use File | Settings | File Templates.
--

local logger = log.open("console")

local c = chan.create()

local counter = 100000

local run_unittest = function (...)
    task.create("unittest.lua", c, ...)
    chan.recv(c)
end

run_unittest("echo_test.lua",counter)

run_unittest("tcp_test.lua",counter)
--
run_unittest("chan_test.lua",counter)









