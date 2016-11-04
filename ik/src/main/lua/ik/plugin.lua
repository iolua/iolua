local cached    = require "ik.cached"
local console   = log.open("console")
local module    = {}

function module.ctor()
    local obj = { tasks = {} }
    return obj
end

function module:loadfrom(ctx)
    -- sync plugin package into locale cached directory
    local path = cached.sync(ctx)

    self:load(path)
end

function module:load(path)
    local filepath = fs.path(path, "src/main/plugin/plugin.lua")

    self.filepath = filepath

    package.path = package.path .. ";" .. fs.path(path,"src/main/plugin/?.lua")

    local env = {}

    for k,v in pairs(_G) do
        env[k] = v
    end

    env.job = {}

    setmetatable(env.job, {
        __newindex = function(_,name,val)
            local task = self.tasks[name]

            if task then
                error(string.format("%s\n\tduplicate task name, see other defined at lines(%d)",filepath, task.lines))
            end

            task = {
                F = val,
                lines = debug.getinfo(2,"lS").currentline
            }

            self.tasks[name] = task
        end,

        __index = function(_,name)
            local task = self.tasks[name]

            if not task then
                error(string.format("%s(%d)\n\t reference undefined task '%s'",filepath, debug.getinfo(2,"lS").currentline, name))
            end

            return task
        end
    })

    local block,err = loadfile(filepath,"t",env)

    if not block then
        error(err)
    end

    block()
end


function module:tasks()
    local tasks = {}

    for name,task in pairs(self.tasks) do
        tasks[name] = {
            lines       = task.lines,
            filepath    = self.filepath,
            prev        = task.prev
        }
    end

    return tasks
end

function module:run(name, ...)
    local task = self.tasks[name]

    if task == nil then
        error(string.format("unknown task %s for plugin %s",name,self.name))
    end

    return task.F(task,...)
end


return module


