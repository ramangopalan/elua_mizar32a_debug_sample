// eLua module for Mizar2 Real Time Clock hardware

#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
#include "platform.h"
#include "lrotable.h"
//#include "lrodefs.h"
#include "platform_conf.h"

//#include "rtc.h"
#include "i2c.h"

// Mizar32 has a real time clock chip on the Ethernet/RTC add-on board
// which may be a DS1337 or may be a PCF8563 part. Both are on the main I2C bus
// but the DS1337 reaponds to 7-bit decimal slave address 104 (8-bit 0xD0, 0xD1)
// and the PCF8563 responds to slave address 81 (0xA2, 0xA3).
// Their functionality is much the same and their register contents are similar
// but their register layouts differ and a few bits mean different things.

// It provides two functions: mizar32.rtc.get() and mizer32.rtc.set(table).
// The first returns a table of lua-like exploded time/date fields:
//  year (1900-2099) month (1-12) day (1-13) hour (0-23) min (0-59) sec (0-59)
// The second accepts the same kind of table, replacing nil values with 0.
// Lua's isdst field is ignored and not returned (so t.isdst is nil==false).
// Both devices have a day-of-week field (with values 1-7 and 0-6 respectively);
// we may one day either use this or just calculate the DOW.

// DS1337 registers: http://datasheets.maxim-ic.com/en/ds/DS1337-DS1337C.pdf p.8
// PCF8563 registers: http://www.nxp.com/documents/data_sheet/PCF8563.pdf p.6

// Slave addresses of the two possible RTC chips
#define DS1337_SLAVE_ADDRESS 104
#define PCF8536_SLAVE_ADDRESS 81

#define TWELVE_HOUR_MODE (1<<6) // 12/24-hour Mode bit in Hour register
#define TWELVE_HOUR_PM (1<<5)   // am/pm bit in hour register

#define AM 0
#define PM 1

#define CONTROL_BIT_RS0  (1<<0) // RS0 bit in control register
#define CONTROL_BIT_RS1  (1<<1) // RS1 bit in control register
#define CONTROL_BIT_SQWE (1<<4) // Square wave enable bit in control register
#define CONTROL_BIT_OUT (1<<7)  // SQW Output value in control register

// Both devices respond up to 100kHz so set maximum I2C frequency.
#define RTC_BUS_FREQUENCY 100000

// Base register for complete time/date readings
#define DS1307_REGISTER_BASE DS1307_REGISTER_SECONDS

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

// sqw_rate -- enum for possible SQW pin output settings
typedef enum sqw_rate {
  SQW_SQUARE_1,
  SQW_SQUARE_4K,
  SQW_SQUARE_8K,
  SQW_SQUARE_32K,
  SQW_LOW,
  SQW_HIGH
} sqw_rate;

// Which type of RTC clock do we have?
static enum rtc_type {
  RTC_DS1337,
  RTC_PCF8536,
  RTC_SYSTEM,	// No RTC present: use the system timer.
} rtc_type;

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

/* enum time_order { */
/*   TIME_SECONDS, // 0 */
/*   TIME_MINUTES, // 1 */
/*   TIME_HOURS,   // 2 */
/*   TIME_DAY,     // 3 */
/*   TIME_DATE,    // 4 */
/*   TIME_MONTH,   // 5 */
/*   TIME_YEAR,    // 6 */
/* }; */

static u8 rtc_slave_address;    // 7-bit I2C slave address of RTC chip
static u8 rtc_registers_start;  // Index in hardware of "seconds" register

/* Initialise the RTC subsystem:
 * - Probe the possible devices to know what kind of RTC we have
 * - If none, do the RTC stuff using the one-second-tick system timer
 */
static void rtc_init()
{
  static u8 rtc_initialized = 0;	// Have we probed it?

  if (rtc_initialized) return;

  if ( i2c_probe( DS1337_SLAVE_ADDRESS ) ) {
    rtc_type = RTC_DS1337;
    rtc_slave_address = DS1337_SLAVE_ADDRESS;
    rtc_registers_start = 0;
  } else if( i2c_probe( PCF8536_SLAVE_ADDRESS ) ) {
    rtc_type = RTC_PCF8536;
    rtc_slave_address = PCF8536_SLAVE_ADDRESS;
    rtc_registers_start = 2;
  } else { 
    rtc_type = RTC_SYSTEM;
  }

  rtc_initialized++;
}

// The names of the Lua time table elements to return, in the order they are
// in the DS1337 register set.
static const char * const fieldnames[] = {
  "sec", "min", "hour", "wday", "day", "month", "year"
};
static const u16 minval[] = {
  0, 0, 0, 1, 1, 1, 1900
};
static const u16 maxval[] = {
  59, 59, 23, 7, 31, 12, 2099
};

// The number of fields in the above tables, also the number of bytes in the
// chip's time-oriented register set.
#define NFIELDS 7

// The order offsets of the fields in the DS1337 register set
#define SEC   0
#define MIN   1
#define HOUR  2
#define WDAY  3
#define DAY   4
#define MONTH 5
#define YEAR  6

// Convert from BCD to a regular integer
static unsigned fromBCD(u8 bcd)
{
   return (bcd & 0xF) + (bcd >> 4) * 10;
}

// Convert from an integer to 2-digit BCD
static u8 toBCD(unsigned n)
{
   return ( ( n / 10 ) << 4 ) | ( n % 10 );
}

static void read_rtc_regs(u8 *regs)
{
  // Read the chip's registers into our buffer
  i2c_send(rtc_slave_address, &rtc_registers_start, 1, false);
  i2c_recv(rtc_slave_address, regs, NFIELDS, true);

  // Convert PCF8536 data format to DS1337 data format
  if( rtc_type == RTC_PCF8536 ) {
    u8 tmp;

    // day-of-week and day-of-month are swapped
    // and day-of-week is 0-6 instead of Lua's 1-7
    tmp = regs[WDAY];           // reads "date" register
    regs[WDAY] = regs[DAY] + 1; // reads "weekday"register"
    regs[DAY] = tmp;
  }
}

static void write_rtc_regs(u8 *regs)
{
  // Convert DS1337 data format to PCF8536 data format
  if( rtc_type == RTC_PCF8536 ) {
    u8 tmp;

    // day-of-week and day-of-month are swapped
    // and day-of-week is 0-6 instead of Lua's 1-7
    tmp = regs[WDAY];
    regs[WDAY] = regs[DAY]; // writes to PCF's "date" register
    regs[DAY] = tmp - 1;    // writes to PCF's weekday register
  }
  // The buffer has a spare byte at the start to poke the register address into
  regs[-1] = rtc_registers_start;
  i2c_send(rtc_slave_address, regs - 1, NFIELDS+1, true);
}

// Read the time from the RTC.
static int rtc_get( lua_State *L )
{
  u8 regs[7];  // seconds to years register contents, read from device

  rtc_init();

  if( rtc_type == RTC_SYSTEM ) {
    // Unimplemented as yet
    return luaL_error( L, "No real-time clock chip present" );
  }

  read_rtc_regs( regs );

  // Construct the table to return the result
  lua_createtable( L, 0, 7 );

  lua_pushstring( L, "sec" );
  lua_pushinteger( L, fromBCD(regs[SEC] & 0x7F) );
  lua_rawset( L, -3 );

  lua_pushstring( L, "min" );
  lua_pushinteger( L, fromBCD(regs[MIN] & 0x7F) );
  lua_rawset( L, -3 );

  lua_pushstring( L, "hour" );
  if( regs[HOUR] & TWELVE_HOUR_MODE )
    lua_pushinteger( L, fromBCD(regs[HOUR] & 0x1F) );
  else
    lua_pushinteger( L, fromBCD(regs[HOUR] & 0x3F) );
  lua_rawset( L, -3 );

  lua_pushstring( L, "wday" );
  lua_pushinteger( L, regs[WDAY] & 0x07 );
  lua_rawset( L, -3 );

  lua_pushstring( L, "day" );
  lua_pushinteger( L, fromBCD(regs[DAY] & 0x3F) );
  lua_rawset( L, -3 );

  lua_pushstring( L, "month" );
  lua_pushinteger( L, fromBCD(regs[MONTH] & 0x1F) );
  lua_rawset( L, -3 );

  lua_pushstring( L, "year" );
  lua_pushinteger( L, fromBCD(regs[YEAR] & 0xFF) + 2000 );           // Year in century
  //+ ( ( regs[MONTH] & 0x80 ) ? 2000 : 1900 ) ); // Century
  lua_rawset( L, -3 );

  return 1;
}

// mizar32.rtc.set()
// Parameter is a table containing fields with the usual Lua time field
// names.  Missing elements are not set and remain the same as they were.

static int rtc_set( lua_State *L )
{
  u8 buf[NFIELDS+1];	// I2C message: First byte is the register address
  u8 *regs = buf + 1;	// The registers
  lua_Integer value;
  int field;         // Which field are we handling (0-6)

  rtc_init();

  if( rtc_type == RTC_SYSTEM ) {
    // Unimplemented as yet
    return luaL_error( L, "No real-time clock chip present" );
  }

  // Read the chip's registers into our buffer so that unspecified fields
  // remain at the same value as before
  read_rtc_regs( regs );

  // Set any values that they specified as table entries
  for (field=0; field<NFIELDS; field++) {
    lua_getfield( L, 1, fieldnames[field] );
    switch( lua_type( L, -1 ) ) {
    case LUA_TNIL:
      // Do not set unspecified fields
      break;

    case LUA_TNUMBER:
    case LUA_TSTRING:
      value = lua_tointeger( L, -1 );
      if (value < minval[field] || value > maxval[field])
        return luaL_error( L, "Time value out of range" );

      // Special cases for some fields
      switch( field ) {
      case SEC: case MIN: case HOUR:
      case WDAY: case DAY:
        regs[field] = toBCD( value );
        break;
      case MONTH:
        // Set new month but preserve century bit in case they don't set the year
        regs[MONTH] = /* ( regs[MONTH] & 0x80 ) | */ toBCD( value );
        break;
      case YEAR:
        if ( value < 2000 ) {
          value -= 1900; //regs[MONTH] &= 0x7F;
        } else {
          value -= 2000; //regs[MONTH] |= 0x80;
        }
        regs[YEAR] = toBCD( value );
        break;
      }
      break;

    default:
      return luaL_error( L, "Time values must be numbers" );
    }
    lua_pop( L, 1 );
  }

  write_rtc_regs( regs );

  return 0;
}

// is12Hour -- check if the DS1307 is in 12-hour mode
static bool rtc_is_12hour( void )
{
  uint8_t ret, reg_address = DS1307_REGISTER_HOURS;

  i2c_send( rtc_slave_address, &reg_address, 1, true );
  i2c_recv( rtc_slave_address, &ret, 1, true );

  return ret & TWELVE_HOUR_MODE;
}

static int rtc_get_mode( lua_State *L )
{
  lua_pushstring( L, rtc_is_12hour() ? "12 Hours" : "24 Hours" );
  return 1;
}

// pm -- Check if 12-hour state is AM or PM
static bool rtc_is_pm( void ) // Read bit 5 in hour byte
{
  uint8_t ret, reg_address = DS1307_REGISTER_HOURS;

  i2c_send( rtc_slave_address, &reg_address, 1, true );
  i2c_recv( rtc_slave_address, &ret, 1, true );

  return ret & TWELVE_HOUR_PM;
}

static int rtc_meridiem( lua_State *L )
{
  if( rtc_is_12hour() )
  {
    lua_pushstring( L, rtc_is_pm() ? "PM" : "AM" );
    return 1;
  }
  else
  {
    return luaL_error( L, "RTC is not configured for 12 Hours mode" );
  }
}

/* Get me a pulse please? */
static int rtc_sqw( lua_State *L )
{
  uint8_t control_reg = 0, buf[2]; int sqw;

  sqw = luaL_checkinteger( L, 1 );
  switch( sqw )
  {
    case SQW_HIGH: control_reg |= CONTROL_BIT_OUT; break;
    case SQW_LOW: /* Do nothing. Just leave at 0. */ break;
    case SQW_SQUARE_1:
    case SQW_SQUARE_4K:
    case SQW_SQUARE_8K:
    case SQW_SQUARE_32K: control_reg |= CONTROL_BIT_SQWE; control_reg |= sqw; break;
    default: return luaL_error( L, "Invalid SQW argument" );
  }

  buf[0] = DS1307_REGISTER_CONTROL; buf[1] = control_reg;
  i2c_send( rtc_slave_address, buf, 2, true );

  return 0;
}

// autoTime -- Fill DS1307 time registers with compiler time/date
static int rtc_auto_time( lua_State *L )
{
  u8 ds1307_time[NFIELDS + 1];
  u8 addr = DS1307_REGISTER_BASE;

  rtc_init();

  if( rtc_type == RTC_SYSTEM ) {
    // Unimplemented as yet
    return luaL_error( L, "No real-time clock chip present" );
  }

  ds1307_time[SEC] = addr;
  ds1307_time[SEC + 1] = toBCD(BUILD_SECOND);
  ds1307_time[MIN + 1] = toBCD(BUILD_MINUTE);
  ds1307_time[HOUR + 1] = BUILD_HOUR;
  if( rtc_is_12hour() )
  {
    uint8_t pmBit = 0;
    if (ds1307_time[HOUR + 1] <= 11)
    {
      if (ds1307_time[HOUR + 1] == 0)
      ds1307_time[HOUR + 1] = 12;
    }
    else
    {
      pmBit = TWELVE_HOUR_PM;
      if (ds1307_time[HOUR + 1] >= 13)
        ds1307_time[HOUR + 1] -= 12;
    }
    ds1307_time[HOUR + 1] = toBCD(ds1307_time[HOUR + 1]);
    ds1307_time[HOUR + 1] |= pmBit;
    ds1307_time[HOUR + 1] |= TWELVE_HOUR_MODE;
  }
  else
  {
    ds1307_time[HOUR + 1] = toBCD(ds1307_time[HOUR + 1]);
  }

  ds1307_time[MONTH + 1] = toBCD(BUILD_MONTH);
  ds1307_time[DAY + 1] = toBCD(BUILD_DATE);
  ds1307_time[YEAR + 1] = toBCD(BUILD_YEAR - 2000); //! Not Y2K (or Y2.1K)-proof :

  // Calculate weekday (from here: http://stackoverflow.com/a/21235587)
  // 0 = Sunday, 6 = Saturday
  int d = BUILD_DATE;
  int m = BUILD_MONTH;
  int y = BUILD_YEAR;
  int weekday = (d+=m<3?y--:y-2,23*m/9+d+4+y/4-y/100+y/400)%7 + 1;
  ds1307_time[WDAY + 1] = toBCD(weekday);

  i2c_send( rtc_slave_address, ds1307_time, NFIELDS + 1, true );

  return 0;
}

// set24Hour -- set (or not) to 24-hour mode) | enable24 defaults to  true
static int rtc_format( lua_State *L )
{
  unsigned enable24;
  uint8_t hour_register;
  u8 buf[NFIELDS+1];	// I2C message: First byte is the register address
  u8 *regs = buf + 1;	// The registers

  rtc_init();

  if( rtc_type == RTC_SYSTEM ) {
    // Unimplemented as yet
    return luaL_error( L, "No real-time clock chip present" );
  }

  // Read the chip's registers into our buffer so that unspecified fields
  // remain at the same value as before
  read_rtc_regs( regs ); hour_register = regs[HOUR];

  enable24 = luaL_checkinteger( L, 1 );
  bool hour12 = hour_register & TWELVE_HOUR_MODE;
  if ((hour12 && !enable24) || (!hour12 && enable24))
    return 0;

  uint8_t old_hour = hour_register;
  if (hour12)
    old_hour &= 0x1F; // Mask out am/pm and 12-hour mode
  old_hour = fromBCD(old_hour); // Convert to decimal
  uint8_t new_hour = old_hour;

  if (enable24)
  {
    bool hour_pm = hour_register & TWELVE_HOUR_PM;
    if ((hour_pm) && (old_hour >= 1)) new_hour += 12;
    else if (!(hour_pm) && (old_hour == 12)) new_hour = 0;
    new_hour = toBCD(new_hour);
  }
  else
  {
    if (old_hour == 0)
      new_hour = 12;
    else if (old_hour >= 13)
      new_hour -= 12;

    new_hour = toBCD(new_hour);
    new_hour |= TWELVE_HOUR_MODE; // Set bit 6 to set 12-hour mode
    if (old_hour >= 12)
      new_hour |= TWELVE_HOUR_PM; // Set PM bit if necessary
  }

  regs[HOUR] = new_hour;
  write_rtc_regs( regs );

  return 0;
}

static int rtc_enable( lua_State *L )
{
  uint8_t second_reg, buf[2];
  i2c_send( rtc_slave_address, 0, 1, true );
  i2c_recv( rtc_slave_address, &second_reg, 1, true );
  second_reg &= ~( 1 << 7 );
  buf[0] = 0; buf[1] = second_reg;
  i2c_send( rtc_slave_address, buf, 2, true );
  return 0;
}

static int rtc_disable( lua_State *L )
{
  uint8_t second_reg, buf[2];
  i2c_send( rtc_slave_address, 0, 1, true );
  i2c_recv( rtc_slave_address, &second_reg, 1, true );
  second_reg |= ( 1 << 7 );
  buf[0] = 0; buf[1] = second_reg;
  i2c_send( rtc_slave_address, buf, 2, true );
  return 0;
}

#define MIN_OPT_LEVEL 2
#include "lrodefs.h"

// mizar32.rtc.*() module function map
const LUA_REG_TYPE rtc_map[] =
{
  { LSTRKEY( "get" ), LFUNCVAL( rtc_get ) },
  { LSTRKEY( "set" ), LFUNCVAL( rtc_set ) },
  { LSTRKEY( "set_mode" ), LFUNCVAL( rtc_format ) },
  { LSTRKEY( "get_mode" ), LFUNCVAL( rtc_get_mode ) },
  { LSTRKEY( "meridiem" ), LFUNCVAL( rtc_meridiem ) },
  { LSTRKEY( "enable" ), LFUNCVAL( rtc_enable ) },
  { LSTRKEY( "disable" ), LFUNCVAL( rtc_disable ) },
  /* Set Elixir build time as new RTC time */
  { LSTRKEY( "autotime" ), LFUNCVAL( rtc_auto_time ) },
  { LSTRKEY( "HOURS_12" ), LNUMVAL( 0 ) },
  { LSTRKEY( "HOURS_24" ), LNUMVAL( 1 ) },
  /* SQW pin values: For pulse output. */
  { LSTRKEY( "sqw" ), LFUNCVAL( rtc_sqw ) },
  { LSTRKEY( "SQW_1" ), LNUMVAL( SQW_SQUARE_1 ) },
  { LSTRKEY( "SQW_4K" ), LNUMVAL( SQW_SQUARE_4K ) },
  { LSTRKEY( "SQW_8K" ), LNUMVAL( SQW_SQUARE_8K ) },
  { LSTRKEY( "SQW_32K" ), LNUMVAL( SQW_SQUARE_32K ) },
  { LSTRKEY( "SQW_LOW" ), LNUMVAL( SQW_LOW ) },
  { LSTRKEY( "SQW_HIGH" ), LNUMVAL( SQW_HIGH ) },
  { LNILKEY, LNILVAL }
};
