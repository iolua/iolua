
local netstat = exec.create("ps","o")

netstat:start("-ef")

local o = netstat:pipe("o")

task.create("exec_print.lua",o)

netstat:wait()

task.sleep(1000)

pipe.close(o)






