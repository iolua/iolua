local logger = log.open("console")

logger:debug(fs.current_path())

assert(fs.exists(fs.current_path()))

assert(fs.file_type(fs.current_path()) == "directory")

fs.create_directory("test")

assert(fs.exists(fs.current_path() .. "/test"))

fs.remove_file("test")

