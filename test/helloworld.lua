local logger = log.open("test")

logger:verb(table.concat(table.pack(...),", "))