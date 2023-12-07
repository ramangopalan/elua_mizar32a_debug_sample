
if (#arg ~= 7) then
  print("rtc-set: syntax: wday, day, month, year, hour, min, sec")
end


for i = 1, #arg do
  print(arg[i])
end

now = { sec=arg[7], min=arg[6], hour=arg[5], wday=arg[1], day=arg[2], month=arg[3], year=arg[4] }
avr32.rtc.set( now )
print( "RTC configured." )
