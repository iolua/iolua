return function(modulename,...)
    local path = fs.parent_path(string.sub(debug.getinfo(1,"lS").source,2))

    path = fs.path(path,"taskL_loader.lua")

    return task.create(path,modulename, ...)
end

