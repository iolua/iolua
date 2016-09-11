local host,port = ...

local client = iolua.sock(2,1,6)

iolua.connect(client, host, port)