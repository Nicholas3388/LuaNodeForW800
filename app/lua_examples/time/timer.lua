-- Create and start a timer

require "wmtime"

myCallback = function()
  print("enter callback")
  print(wmtime.value(timerId)) --output the specific timer current time
end

anotherCallback = function()
  print("enter another callback")
end

timerId = wmtime.create(1000, REPEAT, "myCallback") --wmtime.create(1000, ONCE, "myCallback"); trigger only once
wmtime.start(timerId)

--wmtime.change(timerId, 2000) --Change the timer trigger time to 2000ms. If the timer is not start, it will start immediately when you call change method
--wmtime.callback(timerId, "anotherCallback") --Change the specific timer callback
--wmtime.stop(timerId)
--wmtime.destroy(timerId)
