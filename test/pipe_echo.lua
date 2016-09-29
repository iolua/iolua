--
-- Created by IntelliJ IDEA.
-- User: yayanyang
-- Date: 2016/9/28
-- Time: 上午10:33
-- To change this template use File | Settings | File Templates.
--

local input,output, n = ...

for v = 1,n,1 do
    pipe.write(output,"hello " .. v)
    pipe.read(input,1024)
end

