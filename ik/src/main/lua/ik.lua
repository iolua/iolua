log.console("","console")
log.file("./log","ik",true,"ik")

local logger  = log.open("ik")
local console = log.open("console")
local cli = require('cliargs')


cli:set_name('ik')


cli
    :command('install')
    :action(function(options)
        
    end)

local args, err = cli:parse(table.pack(...))

if not args and err then
    console:error(err)
elseif args then
    print('git with no command')
end





