log.console("","exec_print")
local logger = log.open("exec_print")

local o = ...

while true do
    local ok, buff = pipe.read(o,1024)
    if not ok then
        logger:error(buff)
        return
    end

    if #buff == 0 then return end

    logger:debug("%s",buff)
end

