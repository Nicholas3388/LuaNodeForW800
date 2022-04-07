-- Delay some ticks
-- Note that the Lua run as a task within RTOS. When you invoke delay function, it means that the Lua task will be delay for some ticks.

require "wmsys"
print("before delay")
wmsys.delay(1000) -- delay 1000 ticks
print("delay over")
