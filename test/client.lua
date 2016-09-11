local host,port = ...

local client = iolua.sock(2,1,6)

local future = iolua.connect(client, host, port)

print("future :", future)

coroutine.yield()

print("!!!!!",iolua.wait(future))


-- iolua.connect(client, "sina.com.cn", 80)