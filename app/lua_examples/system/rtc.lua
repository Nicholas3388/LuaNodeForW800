--Get time from RTC, or other features

require "wmrtc"
rtcInterrupt = function() -- must be global function
  print("RTC interrupt")
end
wmrtc.set(2022, 4, 1, 15, 0, 0)
wmrtc.register("rtcInterrupt")
wmrtc.start()

-- wmrtc.stop()
-- year, month, day, hour, minute, second = wmrtc.get() -- Get current time from pmu RTC