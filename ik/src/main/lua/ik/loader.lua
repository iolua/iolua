local config    = require "ik.config"
local console   = log.open("console")
local module    = {}

function module.ctor()
    return {
        external = true, properties = {}, plugins = {}
    }
end

function module:load(path)
    local packagefile = fs.path(path,"package.lua")

    if fs.exists(packagefile) then
        self.external = false

        -- create package.lua execute env
        local env = {}

        env.name = function(name)
            self.Name = name
        end

        env.version =  function(version)
            self.Version = version
        end

        env.plugin =  function(name)

            local plugin = self.plugins[name]

            if plugin then
                    error("%s\n\tduplicate plugin name, see other defined at lines(%d)",packagefile, plugin.lines)
            end


            plugin = {
                name = name ,
                lines = debug.getinfo(2,"lS").currentline
            }

            setmetatable(plugin, {
                __index = {
                    localpath = function(_,path)
                        plugin.localpath = path
                    end,

                    git = function(_, path)
                        plugin.url = path
                    end
                }
            })

            self.plugins[name] = plugin

            return plugin
        end

        env.property = {}

        setmetatable(env.property, {
            __newindex = function(_,name,val)
                local property = self.properties[name]

                if property then
                    error("%s\n\tduplicate property name, see other defined at lines(%d)",packagefile, property.lines)
                end

                property = {
                    value = val,
                    lines = debug.getinfo(2,"lS").currentline
                }

                self.properties[name] = property
            end
        })

        local block,err = loadfile(packagefile,"t",env)

        if not block then
            error(err)
        end

        block()
    end

    console:info( "load package     : %s", path)
    console:debug("external package : %s", self.external)

    return not self.external
end


function module:get()

end

return module