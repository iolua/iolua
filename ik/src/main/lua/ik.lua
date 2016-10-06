local homepath = os.getenv('IK_HOME')

if homepath == nil then
    console:error("env variable IK_HOME not set")
    return
end

log.console("","console")
log.file(homepath .. "/log","ik",true,"ik")



local console = log.open("console")
local logger = log.open("ik")


logger:info("start iolua make system ...")

local cli = require('cliargs')
local cmd = require('ik.cmd')



cli:set_name('ik')

cli
    :command('get')
    :action(function(options)
        cmd.get(homepath,options)
    end)

local args, err = cli:parse(table.pack(...))

if not args and err then
    console:error(err)
elseif args then
    cli:print_help()
end






