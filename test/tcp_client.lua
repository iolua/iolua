local config,n = ...

local client = socket.create(2,1,6)

local ok = client:connect(config.host, config.port)

if ok then
    for v = 1,n,1 do
        client:send("hello " .. v)
        client:recv(1024)
    end
end