local c,n = ...

for v = 1,n,1 do
    chan.send(c,"hello",v, "task")
    chan.recv(c)
end