local logger = log.open("console")
local starttime = os.time()

local c,file = ...

local f = assert(loadfile(file), "file not found: " .. file)

logger:debug("run %s ...",file)

f(table.unpack(table.pack(...),3))

local endtime = os.time()


logger:debug("run %s -- success(%s)",file, endtime - starttime)

chan.send(c,true)