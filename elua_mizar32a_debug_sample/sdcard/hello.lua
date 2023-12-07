print("Hello world!")
pio.pin.setdir(pio.OUTPUT, pio.PB_29)
for i = 1, 10 do
  pio.pin.sethigh(pio.PB_29)
  tmr.delay(0, 1000000)
  pio.pin.setlow(pio.PB_29)
  tmr.delay(0, 1000000)
end
