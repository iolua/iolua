local host,port = ...

local client = iolua.sock(2,1,6)

print(iolua.connect(client, host, port))


-- iolua.connect(client, "sina.com.cn", 80)