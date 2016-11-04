local console   = log.open("console")
local logger    = log.open("ik")
local taskL     = require "ik.taskL"

job.download = function(_,ctx)
    if not exec.lookup('git') then
        console:error("Not found git, you can install it mannual")
        return true
    end

    local git = exec.create('git','ioe')

    if not ctx.url then
        ctx.url = "https://" .. ctx.name
    end

    if fs.exists(ctx.path) then
        fs.remove(ctx.path)
    end

    local dir = fs.parent_path(ctx.path)
    local filename = fs.filename(ctx.path)

    if not fs.exists(dir) then
        fs.create_directory(dir)
    end

    console:info("download package %s with git ...",ctx.name)

    logger:info("git clone\n\turl:%s\n\tdir:%s\n\ttarget:%s",ctx.url,dir,filename)

    git:workdir(dir)

    git:start("clone",ctx.url,filename)

    local input,output,error = git:pipe('ioe')

    taskL('ik.exec_log',input,output)
    taskL('ik.exec_log',input,error)

    git:wait()

    task.sleep(1000)

    pipe.close(input,output,error)

end