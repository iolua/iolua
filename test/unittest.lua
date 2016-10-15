local logger = log.open("console")
local starttime = os.time()

local c,file,counter = ...

local f = assert(loadfile(file), "file not found: " .. file)

logger:debug("run %s ...",file)

f(table.unpack(table.pack(...),3))

local endtime = os.time()


logger:debug("run %s -- success(%s)",file, (endtime - starttime) * 1000000000 / counter)

chan.send(c,true)