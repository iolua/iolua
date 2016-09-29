
local netstat = exec.create("netstat","o")

netstat:start("-an")

local o = netstat:pipe("o")

task.create("exec_print.lua",o)

netstat:wait()

task.sleep(1000)

netstat:start("-an")

local o = netstat:pipe("o")

task.create("exec_print.lua",o)

netstat:wait()

task.sleep(1000)

pipe.close(o)






