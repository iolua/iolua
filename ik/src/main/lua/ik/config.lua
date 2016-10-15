local logger = log.open("ik")

local module = {}

module.homepath = os.getenv('IK_HOME')

if module.homepath == nil then
    error("env variable IK_HOME not set")
end

module.current_dir = fs.current_path()

module.cached_dir = fs.path(module.current_dir, '.iolua')
module.log_dir = fs.path(module.cached_dir, '/log')

for k,v in pairs(module) do
    if k:match('.+_dir$') then
        logger:info("%s: '%s'",k,v)
    end
end

return module