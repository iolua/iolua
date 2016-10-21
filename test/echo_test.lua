local n = ...

local duo_one = {
    chan.create(),chan.create()
}

local duo_two = {
    chan.create(),chan.create()
}


--local logger = log.open("console")
task.create("echo_echo.lua",duo_one,n)
task.create("echo_echo.lua",duo_two,n)
for v = 1,n * 2,1 do
    
   local c = chan.select(duo_one[1],duo_two[1])
    
   chan.recv(c)
   if c == duo_one[1] then
        chan.send(duo_one[2],v)
   else
       chan.send(duo_two[2],v)
   end

end

--chan.close(duo_one[1])
--chan.close(duo_one[2])
--chan.close(duo_two[1])
--chan.close(duo_two[2])
