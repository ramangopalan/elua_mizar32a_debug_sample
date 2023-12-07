
/******************************************************************************
SparkFunDS1307RTC.h
Jim Lindblom @ SparkFun Electronics
original creation date: October 2, 2016
https://github.com/sparkfun/SparkFun_DS1307_RTC_Arduino_Library

Prototypes the DS1307 class. Defines sketch-usable global variables.

Resources:
Wire.h - Arduino I2C Library

Development environment specifics:
Arduino 1.6.8
SparkFun RedBoard
SparkFun Real Time Clock Module (v14)
******************************************************************************/
#ifndef DS1307RTC_H
#define DS1307RTC_H

#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
#include "platform.h"
#include "lrotable.h"
//#include "lrodefs.h"
#include "platform_conf.h"
#include "i2c.h"
#include "ds1307.h"
#include <string.h>

#define DS1307_RTC_ADDRESS 0x68 // DS1307 only has one I2C address - 0x68

#define TWELVE_HOUR_MODE (1<<6) // 12/24-hour Mode bit in Hour register
#define TWELVE_HOUR_PM (1<<5)   // am/pm bit in hour register

#define AM 0
#define PM 1

#define CONTROL_BIT_RS0  (1<<0) // RS0 bit in control register
#define CONTROL_BIT_RS1  (1<<1) // RS1 bit in control register
#define CONTROL_BIT_SQWE (1<<4) // Square wave enable bit in control register
#define CONTROL_BIT_OUT (1<<7)  // SQW Output value in control register

#define TIME_ARRAY_LENGTH 7 // Total number of writable values in device

enum time_order {
  TIME_SECONDS, // 0
  TIME_MINUTES, // 1
  TIME_HOURS,   // 2
  TIME_DAY,     // 3
  TIME_DATE,    // 4
  TIME_MONTH,   // 5
  TIME_YEAR,    // 6
};

// sqw_rate -- enum for possible SQW pin output settings
typedef enum sqw_rate {
  SQW_SQUARE_1,
  SQW_SQUARE_4K,
  SQW_SQUARE_8K,
  SQW_SQUARE_32K,
  SQW_LOW,
  SQW_HIGH
} sqw_rate;

// ds1307_register -- Definition of DS1307 registers
typedef enum ds1307_registers {
  DS1307_REGISTER_SECONDS, // 0x00
  DS1307_REGISTER_MINUTES, // 0x01
  DS1307_REGISTER_HOURS,   // 0x02
  DS1307_REGISTER_DAY,     // 0x03
  DS1307_REGISTER_DATE,    // 0x04
  DS1307_REGISTER_MONTH,   // 0x05
  DS1307_REGISTER_YEAR,    // 0x06
  DS1307_REGISTER_CONTROL  // 0x07
} ds1307_registers;

// Base register for complete time/date readings
#define DS1307_REGISTER_BASE DS1307_REGISTER_SECONDS

// dayIntToStr -- convert day integer to the string
static const char *ds1307_dayIntToStr[7] = {
  "Sunday",
  "Monday",
  "Tuesday",
  "Wednesday",
  "Thursday",
  "Friday",
  "Saturday"	
};

uint8_t ds1307_time[TIME_ARRAY_LENGTH];
bool ds1307_pm;

// dayIntToChar -- convert day integer to character
static const char dayIntToChar[7] = {'U', 'M', 'T', 'W', 'R', 'F', 'S'};
	
// setTime -- Set time and date/day registers of DS1307
bool ds1307_setTime(uint8_t sec, uint8_t min, uint8_t hour,
	     uint8_t day, uint8_t date, uint8_t month, uint8_t year);
// setTime -- Set time and date/day registers of DS1307 (using data array)
bool ds1307_setTime_data_array(uint8_t * time, uint8_t len);
// autoTime -- Set time with compiler time/date
bool ds1307_autoTime();
	
// To set specific values of the clock, use the set____ functions:
bool ds1307_setSecond(uint8_t s);
bool ds1307_setMinute(uint8_t m);
bool ds1307_setHour(uint8_t h);
bool ds1307_setDay(uint8_t d);
bool ds1307_setDate(uint8_t d);
bool ds1307_setMonth(uint8_t mo);
bool ds1307_setYear(uint8_t y);

uint8_t ds1307_BCDtoDEC(uint8_t val);
uint8_t ds1307_DECtoBCD(uint8_t val);

bool ds1307_update(void);

inline uint8_t ds1307_second(void) { return ds1307_BCDtoDEC(ds1307_time[TIME_SECONDS]); };
inline uint8_t ds1307_minute(void) { return ds1307_BCDtoDEC(ds1307_time[TIME_MINUTES]); };
inline uint8_t ds1307_hour(void) { return ds1307_BCDtoDEC(ds1307_time[TIME_HOURS]); };
inline uint8_t ds1307_day(void) { return ds1307_BCDtoDEC(ds1307_time[TIME_DAY]); };
inline const char ds1307_dayChar(void) { return dayIntToChar[ds1307_BCDtoDEC(ds1307_time[TIME_DAY]) - 1]; };
inline const char * ds1307_dayStr(void) { return ds1307_dayIntToStr[ds1307_BCDtoDEC(ds1307_time[TIME_DAY]) - 1]; };
inline uint8_t ds1307_date(void) { return ds1307_BCDtoDEC(ds1307_time[TIME_DATE]); };
inline uint8_t ds1307_month(void) { return ds1307_BCDtoDEC(ds1307_time[TIME_MONTH]);	};
inline uint8_t ds1307_year(void) { return ds1307_BCDtoDEC(ds1307_time[TIME_YEAR]); };
	
// To read a single value at a time, use the get___ functions:
uint8_t ds1307_getSecond(void);
uint8_t ds1307_getMinute(void);
uint8_t ds1307_getHour(void);
uint8_t ds1307_getDay(void);
uint8_t ds1307_getDate(void);
uint8_t ds1307_getMonth(void);
uint8_t ds1307_getYear(void);
	
// is12Hour -- check if the DS1307 is in 12-hour mode | returns true if 12-hour mode
bool ds1307_is12Hour(void);
// pm -- Check if 12-hour state is AM or PM | returns true if PM
bool ds1307_is_pm(void);
	
///////////////////////////////
// SQW Pin Control Functions //
///////////////////////////////
void ds1307_writeSQW(sqw_rate value); // Write SQW pin high, low, or to a set rate
	
/////////////////////////////
// Misc. Control Functions //
/////////////////////////////
void ds1307_enable(void); // Enable the oscillator
void ds1307_disable(void); // Disable the oscillator (no counting!)
	
bool ds1307_set12Hour(bool enable12); // Enable/disable 12-hour mode
bool ds1307_set24Hour(bool enable24); // Enable/disable 24-hour mode
	
bool ds1307_i2cWriteBytes(uint8_t deviceAddress, ds1307_registers reg, uint8_t * values, uint8_t len);
bool ds1307_i2cWriteByte(uint8_t deviceAddress, ds1307_registers reg, uint8_t value);
uint8_t ds1307_i2cReadByte(uint8_t deviceAddress, ds1307_registers reg);
bool ds1307_i2cReadBytes(uint8_t deviceAddress, ds1307_registers reg, uint8_t * dest, uint8_t len);

#endif // DS1307RTC_H
