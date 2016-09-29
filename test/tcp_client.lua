local config,n = ...

local logger = log.open("console")
logger:debug("create client")
local client = socket.create(2,1,6)
logger:debug("create client -- success")

local ok = socket.connect(client,config.host, config.port)

if ok then
    for v = 1,n,1 do
        socket.send(client,"hello " .. v)
        socket.recv(client,1024)
    end
end

socket.close(client)