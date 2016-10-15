local duo,n = ...

local logger = log.open("console")

for v = 1,n,1 do
    chan.send(duo[1],v)

    chan.recv(duo[2])
end

chan.close(duo[1])
chan.close(duo[2])
