local cached = require "ik.cached"
local module = {}

function module.ctor(ctx)
    ctx.tasks = {}
    return ctx
end

function module:setup()
    -- sync plugin package into locale cached directory
    local path = cached.sync(self)

    -- load local cached plugin package

    local filepath = fs.path(path, "src/main/plugin/plugin.lua")

    self.filepath = filepath

    package.path = package.path .. ";" .. fs.path(path,"src/main/plugin/?.lua")

    local env = {}

    for k,v in pairs(_G) do
        env[k] = v
    end

    env.task = {}

    setmetatable(env.task, {
        __newindex = function(_,name,val)
            local task = self.tasks[name]

            if task then
                error("%s\n\tduplicate task name, see other defined at lines(%d)",packagefile, task.lines)
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
                error("%s(%d)\n\t reference undefined task '%s'",packagefile, debug.getinfo(2,"lS").currentline, name)
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

    task.F(task,...)
end


return module


