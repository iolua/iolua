local console = log.open("console")
local path = ...
local packagefile = fs.path(path,"package.lua")


console:debug("load module: %s", fs.path(path))
console:debug("check file : %s", packagefile)

if fs.exists(packagefile) then
    console:debug("check file : %s -- success", packagefile)
else
    console:error("package.lua file not found\n\tdir: %s", path)
end








