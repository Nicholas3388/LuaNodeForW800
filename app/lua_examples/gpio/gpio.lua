-- GPIO example
-- In this example, we show you how to init IO with Lua, and then read/write the pin

require "wmgpio"
-- include 4 params: port, pin, direction, pin attribute
ret=wmgpio.init(PORTB, 8, GPIO_OUT, FLOATING)
if ret==0 then 
  wmgpio.out(PORTB, 8, HIGH) -- write io, output high level
  print(wmgpio.in(PORTB, 8)) -- read io, return pin level
  wmgpio.out(PORTB, 8, LOW) -- write io, output low level
  print(wmgpio.in(PORTB, 8)) -- read io
else 
  print("GPIO init failed")
end