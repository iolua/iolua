local server = iolua.sock(2,1,6)

iolua.bind(server,"localhost",8080)

iolua.listen(server)

iolua.task("client.lua","localhost",8080)

iolua.task("client.lua","sina.com.cn",80)

while true do
    local ok, sock, addr, port = iolua.accept(server)

    if ok then
        iolua.task("connect.lua",sock,addr,port)
    end
end

