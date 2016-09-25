local server = socket.create(2,1,6)
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
        local ok, buff = client:recv(1024)
        client:send(buff)
    end
end
