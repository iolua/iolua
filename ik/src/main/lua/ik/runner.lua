local logger = log.open("ik")

local topSort

topSort = function(self,taskGroup)
    
    if taskGroup.mark == "black" then return end

    if taskGroup.mark == "gray" then

        local errmsg = "DCG detected:"

        local flag = false

        for _,curr in ipairs(self.checkerOfDCG) do

            if curr == taskGroup then flag = true end

            if flag then
                errmsg = string.format("%s\n\t%s ->",errmsg, curr.Name)
            end

        end

        errmsg = string.format("%s\n\t%s",errmsg, taskGroup.Name)

        throw(errmsg)
    end

    local sortGroups = {}

    taskGroup.mark = "gray"

    table.insert(self.checkerOfDCG,taskGroup)

    for _,task in ipairs(taskGroup) do

        if task.prev ~= nil and task.prev ~= "" then
            local prev = self.taskGroups[task.prev]

            if prev == nil then
                error(string.format("\n\t%s(%d) : dependency task '%s' not found !!!!", task.filepath,task.lines, task.prev))
            end

            local childSortGroups = topSort(self, prev)

            if childSortGroups ~= nil then

                for _,taskGroups in ipairs(childSortGroups) do
                    table.insert(sortGroups,taskGroups)
                end

            end
        end
    end

    table.insert(sortGroups,taskGroup)

    table.remove(self.checkerOfDCG,#self.checkerOfDCG)

    taskGroup.mark = "black"

    return sortGroups
end


local function run( ctx, task, ...)

    if not task then return end

    local taskGroups = {}       
    
    for name,task in pairs(ctx.tasks or {}) do
        if taskGroups[name] == nil then
            taskGroups[name] = { name = name, task}
        else
            table.insert(taskGroups[name],task)
        end
    end

    for _, plugin in pairs(ctx.plugins) do
        for name, taskinfo in pairs(plugin:tasks()) do
            local task = {
                F           =   function(_,...)
                                    plugin:run(name, ...)
                                end,

                prev        = taskinfo.prev,

                lines       = taskinfo.lines,

                filepath    = taskinfo.filepath,
            }

            if taskGroups[name] == nil then
                taskGroups[name] = {name = name, task}
            else
                table.insert(taskGroups[name],task)
            end
        end
    end
    
    
    local taskGroup = taskGroups[task]                                        

    if not taskGroup then
        error(string.format("unknown task :%s",task)) 
    end                

    local callstack = topSort({
        checkerOfDCG = {},
        taskGroups = taskGroups
    },taskGroup)

    
    for i, taskgroup in ipairs(callstack) do

        logger:info("invoke task(%s)",taskgroup.Name)

        for _,task in ipairs(taskgroup) do
           
            if i == #callstack then
                if task.F(task,...) then
                    return true
                end
            else
                if task.F(task) then
                    return true
                end
            end
        end

        logger:info("invoke task(%s) -- success",taskgroup.Name)
    end
end

return run