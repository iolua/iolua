local client, host, port = ...

while true do
print(iolua.recv(client, 50))
print("recv ............")
iolua.send(client,"echo")
end