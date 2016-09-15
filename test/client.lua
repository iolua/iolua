local host,port = ...

local client = iolua.sock(2,1,6)

while true do
    local ok  = iolua.connect(client, host, port)

    if ok then break end
end




local i = 0

while true do

    print("<<<======")

    iolua.send(client,"hello world" .. i)

    print("<<<======>>>")

    print(iolua.recv(client,50))

    i = i + 1

end