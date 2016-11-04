local logger = log.open("ik")

local module = {}

module.homepath = os.getenv('IOLUA_HOME')

if module.homepath == nil then
    error("env variable IOLUA_HOME not set")
end

module.builtin_sync_git_path = fs.path(module.homepath,'ik-sync-git')

module.current_dir = fs.current_path()

module.cached_global_dir = fs.path(module.homepath, 'repo')
module.cached_local_dir = fs.path(module.current_dir, '.iolua_cached')
module.log_dir = fs.path(module.current_dir, '.iolua/log')


for k,v in pairs(module) do
    if k:match('.+_dir$') then
        logger:info("%s: '%s'",k,v)
    end
end

return module