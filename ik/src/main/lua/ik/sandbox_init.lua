local c,file = ...

local call = function ( ... )

    local f,err = loadfile(file)

    if not f then
        error(err)
    end

    chan.send(c,true,f(table.unpack(table.pack(...),3)))
end

local ok, err = pcall(call, ...)

if not ok then
    chan.send(c,false,string.format("load sandbox module(%s)\n\terr: %s",file,err))
end

