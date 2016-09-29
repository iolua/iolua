local console = log.open("console")

local input,output = pipe.create()
--
local input2,output2 = pipe.create()

local n = ...

task.create("pipe_echo.lua",input,output2,n)

for _ = 1,n,1 do
    local _, buff = pipe.read(input2,1024)
    pipe.write(output,buff)
end



