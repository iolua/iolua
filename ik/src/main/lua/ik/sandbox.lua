return function ( modulename, ... )
     local loader = {
        reader          = chan.create(), -- read channel
        writer          = chan.create(), -- write channel    
    }

    setmetatable(loader, {
        __index = function(self,name)
            return function(self, ...)
                chan.send(self.writer, name, ...)
                local retargs = table.pack(chan.recv(self.reader))
                
                if retargs[1] then
                    return table.unpack(retargs,2)
                else
                    error(string.format("\n\tcall %s method %s failed, %s",modulename, name, retargs[2]))
                end
            end
        end,

        __gc = function(self)
            
            chan.close(self.reader)
            chan.close(self.writer)
        end
    })

    for path in string.gmatch(package.path, "[^;]+") do
        path = string.gsub(path, "%?%.lua","ik/sandbox_loader.lua")
        if fs.exists(path) then
            task.create(path,loader.reader,loader.writer,modulename, ...)
        end
    end

    return loader
end