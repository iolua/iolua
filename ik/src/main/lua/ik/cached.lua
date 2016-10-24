--
-- Created by IntelliJ IDEA.
-- User: yayan
-- Date: 2016/10/23
-- Time: 17:03
-- To change this template use File | Settings | File Templates.
--
local console   = log.open("console")
local module    = {}

-- sync package by name version and sync downloader
function module.sync(ctx)

    if ctx.sync.name == "localpath" and ctx.url then
        return ctx.url:gsub("%${root}", ctx.load_dir)
    end

end

return module