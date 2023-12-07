/******************************************************************************
SparkFunDS1307RTC.cpp
Jim Lindblom @ SparkFun Electronics
original creation date: October 2, 2016
https://github.com/sparkfun/SparkFun_DS1307_RTC_Arduino_Library

Implementation of DS1307 real time clock functions

Resources:
Wire.h - Arduino I2C Library

Development environment specifics:
Arduino 1.6.8
SparkFun RedBoard
SparkFun Real Time Clock Module (v14)
******************************************************************************/

#include "ds1307.h"

// Parse the __DATE__ predefined macro to generate date defaults:
// __Date__ Format: MMM DD YYYY (First D may be a space if <10)
// <MONTH>
#define BUILD_MONTH_JAN ((__DATE__[0] == 'J') && (__DATE__[1] == 'a')) ? 1 : 0
#define BUILD_MONTH_FEB (__DATE__[0] == 'F') ? 2 : 0
#define BUILD_MONTH_MAR ((__DATE__[0] == 'M') && (__DATE__[1] == 'a') && (__DATE__[2] == 'r')) ? 3 : 0
#define BUILD_MONTH_APR ((__DATE__[0] == 'A') && (__DATE__[1] == 'p')) ? 4 : 0
#define BUILD_MONTH_MAY ((__DATE__[0] == 'M') && (__DATE__[1] == 'a') && (__DATE__[2] == 'y')) ? 5 : 0
#define BUILD_MONTH_JUN ((__DATE__[0] == 'J') && (__DATE__[1] == 'u') && (__DATE__[2] == 'n')) ? 6 : 0
#define BUILD_MONTH_JUL ((__DATE__[0] == 'J') && (__DATE__[1] == 'u') && (__DATE__[2] == 'l')) ? 7 : 0
#define BUILD_MONTH_AUG ((__DATE__[0] == 'A') && (__DATE__[1] == 'u')) ? 8 : 0
#define BUILD_MONTH_SEP (__DATE__[0] == 'S') ? 9 : 0
#define BUILD_MONTH_OCT (__DATE__[0] == 'O') ? 10 : 0
#define BUILD_MONTH_NOV (__DATE__[0] == 'N') ? 11 : 0
#define BUILD_MONTH_DEC (__DATE__[0] == 'D') ? 12 : 0
#define BUILD_MONTH BUILD_MONTH_JAN | BUILD_MONTH_FEB | BUILD_MONTH_MAR | \
  BUILD_MONTH_APR | BUILD_MONTH_MAY | BUILD_MONTH_JUN |			\
  BUILD_MONTH_JUL | BUILD_MONTH_AUG | BUILD_MONTH_SEP |			\
  BUILD_MONTH_OCT | BUILD_MONTH_NOV | BUILD_MONTH_DEC
// <DATE>
#define BUILD_DATE_0 ((__DATE__[4] == ' ') ? 0 : (__DATE__[4] - 0x30))
#define BUILD_DATE_1 (__DATE__[5] - 0x30)
#define BUILD_DATE ((BUILD_DATE_0 * 10) + BUILD_DATE_1)
// <YEAR>
#define BUILD_YEAR (((__DATE__[7] - 0x30) * 1000) + ((__DATE__[8] - 0x30) * 100) + \
                    ((__DATE__[9] - 0x30) * 10)  + ((__DATE__[10] - 0x30) * 1))

// Parse the __TIME__ predefined macro to generate time defaults:
// __TIME__ Format: HH:MM:SS (First number of each is padded by 0 if <10)
// <HOUR>
#define BUILD_HOUR_0 ((__TIME__[0] == ' ') ? 0 : (__TIME__[0] - 0x30))
#define BUILD_HOUR_1 (__TIME__[1] - 0x30)
#define BUILD_HOUR ((BUILD_HOUR_0 * 10) + BUILD_HOUR_1)
// <MINUTE>
#define BUILD_MINUTE_0 ((__TIME__[3] == ' ') ? 0 : (__TIME__[3] - 0x30))
#define BUILD_MINUTE_1 (__TIME__[4] - 0x30)
#define BUILD_MINUTE ((BUILD_MINUTE_0 * 10) + BUILD_MINUTE_1)
// <SECOND>
#define BUILD_SECOND_0 ((__TIME__[6] == ' ') ? 0 : (__TIME__[6] - 0x30))
#define BUILD_SECOND_1 (__TIME__[7] - 0x30)
#define BUILD_SECOND ((BUILD_SECOND_0 * 10) + BUILD_SECOND_1)

// Constructor -- Initialize class variables to 0
void ds1307_init(void)
{
  int i;
  for (i=0; i<TIME_ARRAY_LENGTH; i++)
    {
      ds1307_time[i] = 0;
    }
  ds1307_pm = false;
}

// setTime -- Set time and date/day registers of DS1307
bool ds1307_setTime(uint8_t sec, uint8_t min, uint8_t hour, uint8_t day, uint8_t date, uint8_t month, uint8_t year)
{
  ds1307_time[TIME_SECONDS] = ds1307_DECtoBCD(sec);
  ds1307_time[TIME_MINUTES] = ds1307_DECtoBCD(min);
  ds1307_time[TIME_HOURS] = ds1307_DECtoBCD(hour);
  ds1307_time[TIME_DAY] = ds1307_DECtoBCD(day);
  ds1307_time[TIME_DATE] = ds1307_DECtoBCD(date);
  ds1307_time[TIME_MONTH] = ds1307_DECtoBCD(month);
  ds1307_time[TIME_YEAR] = ds1307_DECtoBCD(year);
	
  return ds1307_setTime_data_array(ds1307_time, TIME_ARRAY_LENGTH);
}

// setTime -- Set time and date/day registers of DS1307 (using data array)
bool ds1307_setTime_data_array(uint8_t * time, uint8_t len)
{
  if (len != TIME_ARRAY_LENGTH)
    return false;
	
  return ds1307_i2cWriteBytes(DS1307_RTC_ADDRESS, DS1307_REGISTER_BASE, time, TIME_ARRAY_LENGTH);
}

// autoTime -- Fill DS1307 time registers with compiler time/date
bool ds1307_autoTime()
{
  ds1307_time[TIME_SECONDS] = ds1307_DECtoBCD(BUILD_SECOND);
  ds1307_time[TIME_MINUTES] = ds1307_DECtoBCD(BUILD_MINUTE);
  ds1307_time[TIME_HOURS] = BUILD_HOUR;
  if (ds1307_is12Hour())
    {
      uint8_t pmBit = 0;
      if (ds1307_time[TIME_HOURS] <= 11)
	{
	  if (ds1307_time[TIME_HOURS] == 0)
	    ds1307_time[TIME_HOURS] = 12;
	}
      else
	{
	  pmBit = TWELVE_HOUR_PM;
	  if (ds1307_time[TIME_HOURS] >= 13)
	    ds1307_time[TIME_HOURS] -= 12;
	}
      ds1307_DECtoBCD(ds1307_time[TIME_HOURS]);
      ds1307_time[TIME_HOURS] |= pmBit;
      ds1307_time[TIME_HOURS] |= TWELVE_HOUR_MODE;
    }
  else
    {
      ds1307_DECtoBCD(ds1307_time[TIME_HOURS]);
    }
	
  ds1307_time[TIME_MONTH] = ds1307_DECtoBCD(BUILD_MONTH);
  ds1307_time[TIME_DATE] = ds1307_DECtoBCD(BUILD_DATE);
  ds1307_time[TIME_YEAR] = ds1307_DECtoBCD(BUILD_YEAR - 2000); //! Not Y2K (or Y2.1K)-proof :
	
  // Calculate weekday (from here: http://stackoverflow.com/a/21235587)
  // 0 = Sunday, 6 = Saturday
  int d = BUILD_DATE;
  int m = BUILD_MONTH;
  int y = BUILD_YEAR;
  int weekday = (d+=m<3?y--:y-2,23*m/9+d+4+y/4-y/100+y/400)%7 + 1;
  ds1307_time[TIME_DAY] = ds1307_DECtoBCD(weekday);
	
  ds1307_setTime_data_array(ds1307_time, TIME_ARRAY_LENGTH);

  return true;
}

// update -- Read all time/date registers and update the _time array
bool ds1307_update(void)
{
  uint8_t rtcReads[7]; int i;
	
  if (ds1307_i2cReadBytes(DS1307_RTC_ADDRESS, DS1307_REGISTER_SECONDS, rtcReads, 7))
    {
      for (i=0; i<TIME_ARRAY_LENGTH; i++)
	{
	  ds1307_time[i] = rtcReads[i];
	}
		
      ds1307_time[TIME_SECONDS] &= 0x7F; // Mask out CH bit
		
      if (ds1307_time[TIME_HOURS] & TWELVE_HOUR_MODE)
	{
	  if (ds1307_time[TIME_HOURS] & TWELVE_HOUR_PM)
	    ds1307_pm = true;
	  else
	    ds1307_pm = false;
	  ds1307_time[TIME_HOURS] &= 0x1F; // Mask out 24-hour bit from hours
	}
		
      return true;
    }
  else
    {
      return false;
    }	
}

// getSecond -- read/return seconds register of DS1307
uint8_t ds1307_getSecond(void)
{
  ds1307_time[TIME_SECONDS] = ds1307_i2cReadByte(DS1307_RTC_ADDRESS, DS1307_REGISTER_SECONDS);
  ds1307_time[TIME_SECONDS] &= 0x7F; // Mask out CH bit

  return ds1307_BCDtoDEC(ds1307_time[TIME_SECONDS]);
}

// getMinute -- read/return minutes register of DS1307
uint8_t ds1307_getMinute(void)
{
  ds1307_time[TIME_MINUTES] = ds1307_i2cReadByte(DS1307_RTC_ADDRESS, DS1307_REGISTER_MINUTES);

  return ds1307_BCDtoDEC(ds1307_time[TIME_MINUTES]);	
}

// getHour -- read/return hour register of DS1307
uint8_t ds1307_getHour(void)
{
  uint8_t hourRegister = ds1307_i2cReadByte(DS1307_RTC_ADDRESS, DS1307_REGISTER_HOURS);
	
  if (hourRegister & TWELVE_HOUR_MODE)
    hourRegister &= 0x1F; // Mask out am/pm, 24-hour bit
  ds1307_time[TIME_HOURS] = hourRegister;

  return ds1307_BCDtoDEC(ds1307_time[TIME_HOURS]);
}

// getDay -- read/return day register of DS1307
uint8_t ds1307_getDay(void)
{
  ds1307_time[TIME_DAY] = ds1307_i2cReadByte(DS1307_RTC_ADDRESS, DS1307_REGISTER_DAY);

  return ds1307_BCDtoDEC(ds1307_time[TIME_DAY]);		
}

// getDate -- read/return date register of DS1307
uint8_t ds1307_getDate(void)
{
  ds1307_time[TIME_DATE] = ds1307_i2cReadByte(DS1307_RTC_ADDRESS, DS1307_REGISTER_DATE);

  return ds1307_BCDtoDEC(ds1307_time[TIME_DATE]);		
}

// getMonth -- read/return month register of DS1307
uint8_t ds1307_getMonth(void)
{
  ds1307_time[TIME_MONTH] = ds1307_i2cReadByte(DS1307_RTC_ADDRESS, DS1307_REGISTER_MONTH);

  return ds1307_BCDtoDEC(ds1307_time[TIME_MONTH]);	
}

// getYear -- read/return year register of DS1307
uint8_t ds1307_getYear(void)
{
  ds1307_time[TIME_YEAR] = ds1307_i2cReadByte(DS1307_RTC_ADDRESS, DS1307_REGISTER_YEAR);

  return ds1307_BCDtoDEC(ds1307_time[TIME_YEAR]);		
}

// setSecond -- set the second register of the DS1307
bool ds1307_setSecond(uint8_t s)
{
  if (s <= 59)
    {
      uint8_t _s = ds1307_DECtoBCD(s);
      return ds1307_i2cWriteByte(DS1307_RTC_ADDRESS, DS1307_REGISTER_SECONDS, _s);
    }
	
  return false;
}

// setMinute -- set the minute register of the DS1307
bool ds1307_setMinute(uint8_t m)
{
  if (m <= 59)
    {
      uint8_t _m = ds1307_DECtoBCD(m);
      return ds1307_i2cWriteByte(DS1307_RTC_ADDRESS, DS1307_REGISTER_MINUTES, _m);		
    }
	
  return false;
}

// setHour -- set the hour register of the DS1307
bool ds1307_setHour(uint8_t h)
{
  if (h <= 23)
    {
      uint8_t _h = ds1307_DECtoBCD(h);
      return ds1307_i2cWriteByte(DS1307_RTC_ADDRESS, DS1307_REGISTER_HOURS, _h);
    }
	
  return false;
}

// setDay -- set the day register of the DS1307
bool ds1307_setDay(uint8_t d)
{
  if ((d >= 1) && (d <= 7))
    {
      uint8_t _d = ds1307_DECtoBCD(d);
      return ds1307_i2cWriteByte(DS1307_RTC_ADDRESS, DS1307_REGISTER_DAY, _d);
    }	
	
  return false;
}

// setDate -- set the date register of the DS1307
bool ds1307_setDate(uint8_t d)
{
  if (d <= 31)
    {
      uint8_t _d = ds1307_DECtoBCD(d);
      return ds1307_i2cWriteByte(DS1307_RTC_ADDRESS, DS1307_REGISTER_DATE, _d);
    }	
	
  return false;
}

// setMonth -- set the month register of the DS1307
bool ds1307_setMonth(uint8_t mo)
{
  if ((mo >= 1) && (mo <= 12))
    {
      uint8_t _mo = ds1307_DECtoBCD(mo);
      return ds1307_i2cWriteByte(DS1307_RTC_ADDRESS, DS1307_REGISTER_MONTH, _mo);
    }	
	
  return false;	
}

// setYear -- set the year register of the DS1307
bool ds1307_setYear(uint8_t y)
{
  if (y <= 99)
    {
      uint8_t _y = ds1307_DECtoBCD(y);
      return ds1307_i2cWriteByte(DS1307_RTC_ADDRESS, DS1307_REGISTER_YEAR, _y);
    }	
	
  return false;	
}

// set12Hour -- set (or not) to 12-hour mode) | enable12 defaults to  true
bool ds1307_set12Hour(bool enable12)
{
  if (enable12)
    ds1307_set24Hour(false);
  else
    ds1307_set24Hour(true);

  return true;
}

// set24Hour -- set (or not) to 24-hour mode) | enable24 defaults to  true
bool ds1307_set24Hour(bool enable24)
{
  uint8_t hourRegister = ds1307_i2cReadByte(DS1307_RTC_ADDRESS, DS1307_REGISTER_HOURS);
	
  bool hour12 = hourRegister & TWELVE_HOUR_MODE;
  if ((hour12 && !enable24) || (!hour12 && enable24))
    return true;
	
  uint8_t oldHour = hourRegister & 0x1F; // Mask out am/pm and 12-hour mode
  oldHour = ds1307_BCDtoDEC(oldHour); // Convert to decimal
  uint8_t newHour = oldHour;
	
  if (enable24)
    {
      bool hourPM = hourRegister & TWELVE_HOUR_PM;
      if ((hourPM) && (oldHour >= 1)) newHour += 12;
      else if (!(hourPM) && (oldHour == 12)) newHour = 0;
      newHour = ds1307_DECtoBCD(newHour);
    }
  else
    {
      if (oldHour == 0) 
	newHour = 12;
      else if (oldHour >= 13)
	newHour -= 12;
		
      newHour = ds1307_DECtoBCD(newHour);
      newHour |= TWELVE_HOUR_MODE; // Set bit 6 to set 12-hour mode
      if (oldHour >= 12)
	newHour |= TWELVE_HOUR_PM; // Set PM bit if necessary
    }
	
  return ds1307_i2cWriteByte(DS1307_RTC_ADDRESS, DS1307_REGISTER_HOURS, newHour);
}

// is12Hour -- check if the DS1307 is in 12-hour mode
bool ds1307_is12Hour(void)
{
  uint8_t hourRegister = ds1307_i2cReadByte(DS1307_RTC_ADDRESS, DS1307_REGISTER_HOURS);
	
  return hourRegister & TWELVE_HOUR_MODE;
}

// pm -- Check if 12-hour state is AM or PM
bool ds1307_is_pm(void) // Read bit 5 in hour byte
{
  uint8_t hourRegister = ds1307_i2cReadByte(DS1307_RTC_ADDRESS, DS1307_REGISTER_HOURS);
	
  return hourRegister & TWELVE_HOUR_PM;	
}

// enable -- enable the DS1307's oscillator
void ds1307_enable(void)  // Write 0 to CH bit
{
  uint8_t secondRegister = ds1307_i2cReadByte(DS1307_RTC_ADDRESS, DS1307_REGISTER_SECONDS);
	
  secondRegister &= ~(1<<7);
	
  ds1307_i2cWriteByte(DS1307_RTC_ADDRESS, DS1307_REGISTER_SECONDS, secondRegister);
}

// disable -- disable the DS1307's oscillator
void ds1307_disable(void) // Write 1 to CH bit
{
  uint8_t secondRegister = ds1307_i2cReadByte(DS1307_RTC_ADDRESS, DS1307_REGISTER_SECONDS);
	
  secondRegister |= (1<<7);
	
  ds1307_i2cWriteByte(DS1307_RTC_ADDRESS, DS1307_REGISTER_SECONDS, secondRegister);
}
	
// writeSQW -- Set the SQW pin high, low, or to one of the square wave frequencies
void ds1307_writeSQW(sqw_rate value)
{
  uint8_t controlRegister = 0;
  if (value == SQW_HIGH)
    {
      controlRegister |= CONTROL_BIT_OUT;
    }
  else if (value == SQW_LOW)
    {
      // Do nothing, just leave 0
    }
  else
    {
      controlRegister |= CONTROL_BIT_SQWE;	
      controlRegister |= value;
    }
	
  ds1307_i2cWriteByte(DS1307_RTC_ADDRESS, DS1307_REGISTER_CONTROL, controlRegister);
}

// BCDtoDEC -- convert binary-coded decimal (BCD) to decimal
uint8_t ds1307_BCDtoDEC(uint8_t val)
{
  return ( ( val / 0x10) * 10 ) + ( val % 0x10 );
}

// BCDtoDEC -- convert decimal to binary-coded decimal (BCD)
uint8_t ds1307_DECtoBCD(uint8_t val)
{
  return ( ( val / 10 ) * 0x10 ) + ( val % 10 );
}

// i2cWriteBytes -- write a set number of bytes to an i2c device, incrementing from a register
bool ds1307_i2cWriteBytes(uint8_t deviceAddress, ds1307_registers reg, uint8_t * values, uint8_t len)
{
  uint8_t reg_address = reg;
  i2c_send(deviceAddress, &reg_address, 1, false);
  i2c_send(deviceAddress, values, len, true);
  return true;
  /* Wire.beginTransmission(deviceAddress); */
  /* Wire.write(reg); */
  /* for (int i=0; i<len; i++) */
  /*   { */
  /*     Wire.write(values[i]); */
  /*   } */
  /* Wire.endTransmission(); */
	
  /* return true; */
}

// i2cWriteByte -- write a byte value to an i2c device's register
bool ds1307_i2cWriteByte(uint8_t deviceAddress, ds1307_registers reg, uint8_t value)
{
  uint8_t reg_address = reg;
  i2c_send(deviceAddress, &reg_address, 1, false);
  i2c_send(deviceAddress, &value, 1, true);
  return true;
  /* Wire.beginTransmission(deviceAddress); */
  /* Wire.write(reg); */
  /* Wire.write(value); */
  /* Wire.endTransmission(); */
	
  /* return true; */
}

// i2cReadByte -- read a byte from an i2c device's register
uint8_t ds1307_i2cReadByte(uint8_t deviceAddress, ds1307_registers reg)
{
  uint8_t readTemp;
  uint8_t reg_address = reg;
  i2c_send(deviceAddress, &reg_address, 1, true);
  i2c_recv(deviceAddress, &readTemp, 1, true);
  return readTemp;
	
  /* Wire.beginTransmission(deviceAddress); */
  /* Wire.write(reg); */
  /* Wire.endTransmission(); */

  /* Wire.requestFrom(deviceAddress, (uint8_t) 1); */
	
  /* return Wire.read(); */
}

// i2cReadBytes -- read a set number of bytes from an i2c device, incrementing from a register
bool ds1307_i2cReadBytes(uint8_t deviceAddress, ds1307_registers reg, uint8_t * dest, uint8_t len)
{
  uint8_t reg_address = reg;
  i2c_send(deviceAddress, &reg_address, 1, true);
  i2c_recv(deviceAddress, dest, len, true);
  return true;
  /* Wire.beginTransmission(deviceAddress); */
  /* Wire.write(reg); */
  /* Wire.endTransmission(); */

  /* Wire.requestFrom(deviceAddress, len); */
  /* for (int i=0; i<len; i++) */
  /*   { */
  /*     dest[i] = Wire.read(); */
  /*   } */
  
  /* return true;   */
}

