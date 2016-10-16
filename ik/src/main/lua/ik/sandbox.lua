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

    local path = fs.parent_path(string.sub(debug.getinfo(1,"lS").source,2))

    path = fs.path(path,"sandbox_loader.lua")

    task.create(path,loader.reader,loader.writer,modulename, ...)
    
    return loader
end