-- Get ticks, which is the ticks since the system started

require "wmsys"
t=wmsys.ticks()
print(t)
print(wmsys.version()) -- print the LuaNode version
