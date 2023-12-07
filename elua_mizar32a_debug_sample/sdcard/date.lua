
local days = { "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat" }
local t = avr32.rtc.get()

avr32.lcd.clear()
avr32.lcd.goto( 1, 1 )
avr32.lcd.print( string.format("%s, %02d:%02d:%02d",
                  days[t.wday], t.hour, t.min, t.sec ) )
avr32.lcd.goto( 2, 1 )
avr32.lcd.print( string.format("%02d/%02d/%04d",
                     t.day, t.month, t.year ) )

print( string.format( "%s, %02d/%02d/%04d, %02d:%02d:%02d",
       days[t.wday], t.day, t.month, t.year, t.hour, t.min, t.sec ) )
