log.console("","exec_print")
local logger = log.open("exec_print")

local o = ...

while true do
    local ok, buff = pipe.read(o,1024)
    if not ok then
        logger:error(buff)
        return
    end

    if #buff < 1024 then
        logger:debug("test :%s",buff)
        return
    end

    logger:debug("test :%s",buff)
end

