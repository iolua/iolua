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

if ok then
    for v = 1,n,1 do
        local ok, buff = client:recv(1024)
        client:send(buff)
    end

    client:close()
end

server:close()
