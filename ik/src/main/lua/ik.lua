log.console("","console","iolua")
local console = log.open("console")

local homepath = os.getenv('IOLUA_HOME')

if homepath == nil then
    console:error("env variable IOLUA_HOME not set")
    return
end


log.file(homepath .. "/log","ik",true,"ik")


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






