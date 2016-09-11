local host,port = ...

local client = iolua.sock(2,1,6)

print(iolua.connect(client, host, port))


local i = 0

while true do

iolua.send(client,"hello world" .. i)

print(iolua.recv(client,50))

i = i + 1

end