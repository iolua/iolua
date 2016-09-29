local logger = log.open("console")
logger:debug("create server")
local server = socket.create(2,1,6)
logger:debug("create server -- success")
local n = ...
local config = {
    host = "localhost",port = 1812
}

socket.bind(server,config.host, config.port)

socket.listen(server)

task.create("tcp_client.lua",config,n)

local ok, client = socket.accept(server)

if ok then
    for _ = 1,n,1 do
        local ok, buff = socket.recv(client,1024)
        socket.send(client,buff)
    end

    socket.close(client)
end

socket.close(server)
