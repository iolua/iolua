local console   = log.open("console")
local sandbox   = require "ik.sandbox"
local module    = {}

local split_url = function(url)

    local result = {}

    for w in string.gmatch(url,"[^:]+") do
        table.insert(result,w)
    end

    return table.unpack(result)
end

function module.ctor()
    return {
        loaded = false,external = true, properties = {}, plugins = {}, tasks = {}
    }
end


function module:load(path)
    local packagefile = fs.path(path,"package.lua")
    self.path = path

    if fs.exists(packagefile) then
        self.external = false

        -- create package.lua execute env
        local env = { }

        for k,v in pairs(_G) do
            env[k] = v
        end

        env.name = function(name)
            self.Name = name
        end

        env.version =  function(version)
            self.Version = version
        end

        -- create plugin DSL 
        env.plugin =  function(url)

            local name, version = split_url(url)

            local plugin = self.plugins[name]

            if plugin then
                    error(string.format("%s\n\tduplicate plugin name, see other defined at lines(%d)",packagefile, plugin.lines))
            end


            plugin = {
                name    = name ,
                version = version,
                lines   = debug.getinfo(2,"lS").currentline
            }

            setmetatable(plugin, {
                __index = {
                    url = function(_,path)
                        plugin.__url = path

                        return plugin
                    end,

                    sync = function(_, sync)
                        local name, version = split_url(sync)

                        plugin.__sync = {
                            name    = name,
                            version = version
                        }

                        return plugin
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

        env.task = {}

        setmetatable(env.task, {
            __newindex = function(_,name,val)
                local task = self.tasks[name]

                if task then
                    error("%s\n\tduplicate task name, see other defined at lines(%d)",packagefile, task.lines)
                end

                task = {
                    F           = val,
                    filepath    = packagefile,
                    lines       = debug.getinfo(2,"lS").currentline
                }

                self.tasks[name] = task
            end,

            __index = function(_,name)  
                local task = self.tasks[name]

                if not task then
                    error("%s(%d)\n\t reference undefined task '%s'",packagefile, debug.getinfo(2,"lS").currentline, name)
                end

                return task
            end
        })

        local block,err = loadfile(packagefile,"t",env)

        if not block then
            error(err)
        end

        block()
    end

    console:info( "/* ")
    console:info( "/* package: %s", path)
    console:info( "/* external package: %s", self.external)

    for name in pairs(self.plugins) do
        console:info( "/* plugin: %s", name)
    end

    console:info( "/* ")

    return not self.external
end

-- call plugin setup
function module:setup()

    local plugins = {}

    for name,plugin in pairs(self.plugins) do

        local rootloader = sandbox("ik.plugin", {
            name        = plugin.name,
            version     = plugin.version,
            sync        = plugin.__sync,
            url         = plugin.__url,
            load_dir    = self.path,
        })

        rootloader:setup()

        plugins[name] = rootloader
    end

    self.plugins = plugins
end

-- run package task with args
function module:run(path, task, ...)
    
    if not self.loaded then
        self:load(path)
        self:setup()
    end

    local runner = require "ik.runner"

    runner(self, task, ...)
end

return module