local logger = log.open("console")
logger:debug("create server")
local server = socket.create(2,1,6)
logger:debug("create server -- success")
local n = ...
local config = {
    host = "localhost",port = 1812
}

server:bind(config.host, config.port)

server:listen()

task.create("tcp_client.lua",config,n)

local ok, client,host,port = server:accept()

print(ok,client,host,port)

if ok then
    for v = 1,n,1 do
        print("recv")
        local ok, buff = client:recv(1024)
        print("recv -- success")
        client:send(buff)
    end
end
