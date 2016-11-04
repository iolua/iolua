log.console("","console","iolua","io_service")
local console = log.open("console")

local homepath = os.getenv('IOLUA_HOME')

if homepath == nil then
    console:error("env variable IOLUA_HOME not set")
    return
end

log.file(homepath .. "/log","ik",true,"ik","iolua")


local logger = log.open("ik")


logger:info("start iolua make system ...")

local cli = require('cliargs')
local cmd = require('ik.cmd')
local config  = require "ik.config"





cli:set_name('ik')

cli
    :command('get')
    :action(function(options)
        cmd.get(config.current_dir, table.unpack(options))
    end)

cli
    :command('run','Run ik task')
    :argument('task', 'The task name')
    :splat('args', 'Input args',nil, 99)
    :action(function(options)
        cmd.run(config.current_dir, options.task,table.unpack(options.args))
    end)

local args, err = cli:parse(table.pack(...))

if not args and err then
    console:error(err)
elseif args then
    cli:print_help()
end






