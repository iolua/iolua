
local console   = log.open("console")
local config    = require "ik.config"
local runner    = require "ik.runner"
local sandbox   = require "ik.sandbox"
local module    = {}

-- sync package by name version and sync downloader
function module.sync(ctx)

    if ctx.sync and ctx.sync.name == "localpath" and ctx.url then
        return ctx.url:gsub("%${root}", ctx.load_dir)
    end

    -- check L1, L2 cached

    local sync_path

    if not ctx.sync then
        sync_path = config.builtin_sync_git_path
    end

    local sync = module.load_sync(sync_path)

    local path = fs.path(config.cached_global_dir,ctx.name,ctx.version or "current")

    if runner(sync, "download", {
        name            = ctx.name,
        version         = ctx.version,
        url             = ctx.url,
        path            = path,
    }) then
        error(string.format("download with [%s] error",sync_path))
    end

    return path
end

function module.load_sync(path)

    local rootloader = sandbox("ik.plugin")

    rootloader:load(path)

    return {
        plugins = {
            __sync = rootloader
        }
    }
end

return module