-- How to use watchdog

require "wmsys"

wmsys.watchdog_init(10000000) -- trigger watchdog after 10 seconds, the parameter is time with microseconds
wmsys.watchdog_feed()
wmsys.watchdot_deinit()