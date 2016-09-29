local logger = log.open("console")

logger:debug(fs.current_path())

assert(fs.exists(fs.current_path()))

