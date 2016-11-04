local logger    = log.open("ik")

return function(_,output)
    while true do

        local ok, buff = pipe.read(output,4024)

        if not ok then
            logger:error(buff)
            return
        end

        if #buff == 0 then
            print("========")
            return
        end

        logger:debug("%s",buff)
    end
end

