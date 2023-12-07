// Lua board configuration file, automatically generated

#ifndef __GENERATED_MIZAR32A_H__
#define __GENERATED_MIZAR32A_H__

#include <stddef.h>

////////////////////////////////////////////////////////////////////////////////
// Configuration for section 'macros'

#define BOARD_SPI0_SCK_PIN               AVR32_PIN_PA13
#define BOARD_SPI0_SCK_PIN_FUNCTION      0
#define BOARD_SPI0_MISO_PIN              AVR32_PIN_PA11
#define BOARD_SPI0_MISO_PIN_FUNCTION     0
#define BOARD_SPI0_MOSI_PIN              AVR32_PIN_PA12
#define BOARD_SPI0_MOSI_PIN_FUNCTION     0
#define BOARD_SPI0_CS_PIN                AVR32_PIN_PA10
#define BOARD_SPI0_CS_PIN_FUNCTION       0
#define BOARD_SPI1_SCK_PIN               AVR32_PIN_PA15
#define BOARD_SPI1_SCK_PIN_FUNCTION      1
#define BOARD_SPI1_MISO_PIN              AVR32_PIN_PA17
#define BOARD_SPI1_MISO_PIN_FUNCTION     1
#define BOARD_SPI1_MOSI_PIN              AVR32_PIN_PA16
#define BOARD_SPI1_MOSI_PIN_FUNCTION     1
#define BOARD_SPI1_CS_PIN                AVR32_PIN_PA14
#define BOARD_SPI1_CS_PIN_FUNCTION       1

////////////////////////////////////////////////////////////////////////////////
// Configuration for section 'components'

// Configuration for element 'mmcfs'
#define MMCFS_CS_PORT                    0
#define MMCFS_CS_PIN                     SD_MMC_SPI_NPCS_PIN
#define MMCFS_SPI_NUM                    4
#define BUILD_MMCFS

// Configuration for element 'adc'
#define ADC_BUF_SIZE                     1
#define BUILD_ADC

// Configuration for element 'cints'
#define BUILD_C_INT_HANDLERS

// Configuration for element 'avr32_rtc'
#define BUILD_RTC

// Configuration for element 'sercon'
#define CON_FLOW_TYPE                    PLATFORM_UART_FLOW_NONE
#define CON_UART_SPEED                   115200
#define CON_TIMER_ID                     PLATFORM_TIMER_SYS_ID
#define CON_UART_ID                      CDC_UART_ID
#define BUILD_CON_GENERIC

// Configuration for element 'xmodem'
#define CON_UART_XMODEM_ID               0
#define BUILD_XMODEM

// Configuration for element 'luaints'
#define PLATFORM_INT_QUEUE_LOG_SIZE      5
#define BUILD_LUA_INT_HANDLERS

// Configuration for element 'avr32_lcd'
#define BUILD_LCD

// Configuration for element 'term'
#define TERM_LINES                       25
#define TERM_COLS                        80
#define BUILD_TERM

// Configuration for element 'cdc'
#define BUILD_USB_CDC

// Configuration for element 'linenoise'
#define LINENOISE_HISTORY_SIZE_LUA       50
#define LINENOISE_HISTORY_SIZE_SHELL     10
#define BUILD_LINENOISE

// Configuration for element 'shell'
#define SHELL_COMMAND_LIST\
  { "exit", NULL },\
  { "type", shell_cat },\
  { "lua", shell_lua },\
  { "dir", shell_ls },\
  { "clear", shell_clear },\
  { "ed", shell_ed },\
  { "cat", shell_cat },\
  { "wofmt", shell_wofmt },\
  { "ver", shell_ver },\
  { "ls", shell_ls },\
  { "help", shell_help },\
  { "rm", shell_adv_rm },\
  { "mkdir", shell_mkdir },\
  { "cp", shell_cp },\
  { "mv", shell_adv_mv },\
  { "recv", shell_recv },\
  { NULL, NULL }
#define BUILD_ADVANCED_SHELL
#define BUILD_SHELL

////////////////////////////////////////////////////////////////////////////////
// Configuration for section 'config'

// Configuration for element 'ram'
#define MEM_START_ADDRESS                { ( u32 )( INTERNAL_RAM1_FIRST_FREE ),( ( u32 )( SDRAM + ELUA_FIRMWARE_SIZE ) ) }
#define MEM_END_ADDRESS                  { ( u32 )( INTERNAL_RAM1_LAST_FREE ),( ( u32 )( SDRAM + ELUA_FIRMWARE_SIZE ) + ( u32 )( SDRAM_SIZE - ELUA_FIRMWARE_SIZE ) - 1 ) }

// Configuration for element 'vtmr'
#define VTMR_FREQ_HZ                     10
#define VTMR_NUM_TIMERS                  4

///////////////////////////////////////////////////////////////////////////////
// Static sanity checks and additional defines

#if defined( ELUA_BOOT_RPC ) && !defined( BUILD_RPC )
#define BUILD_RPC
#endif

#if defined( BUILD_LUA_INT_HANDLERS ) || defined( BUILD_C_INT_HANDLERS )
  #define BUILD_INT_HANDLERS

  #ifndef INT_TMR_MATCH
  #define INT_TMR_MATCH         ELUA_INT_INVALID_INTERRUPT
  #endif
#endif // #if defined( BUILD_LUA_INT_HANDLERS ) || defined( BUILD_C_INT_HANDLERS )

#ifndef VTMR_NUM_TIMERS
#define VTMR_NUM_TIMERS       0
#endif // #ifndef VTMR_NUM_TIMERS

#ifndef SERMUX_FLOW_TYPE
#define SERMUX_FLOW_TYPE      PLATFORM_UART_FLOW_NONE
#endif

#ifndef CON_FLOW_TYPE
#define CON_FLOW_TYPE         PLATFORM_UART_FLOW_NONE
#endif

#ifndef CON_TIMER_ID
#define CON_TIMER_ID          PLATFORM_TIMER_SYS_ID
#endif

#ifndef RFS_FLOW_TYPE
#define RFS_FLOW_TYPE         PLATFORM_UART_FLOW_NONE
#endif

#ifdef ELUA_BOOT_RPC
  #ifndef RPC_UART_ID
    #define RPC_UART_ID       CON_UART_ID
  #endif

  #ifndef RPC_TIMER_ID
    #define RPC_TIMER_ID      PLATFORM_TIMER_SYS_ID
  #endif

  #ifndef RPC_UART_SPEED
    #define RPC_UART_SPEED    CON_UART_SPEED
  #endif
#endif // #ifdef ELUA_BOOT_RPC

#if ( defined( BUILD_RFS ) || defined( BUILD_SERMUX ) || defined( CON_BUF_SIZE ) || defined ( CDC_BUF_SIZE ) ) && !defined( BUF_ENABLE_UART )
#define BUF_ENABLE_UART
#endif

#if defined( ADC_BUF_SIZE ) && !defined( BUF_ENABLE_ADC )
#define BUF_ENABLE_ADC
#endif

#ifndef CPU_FREQUENCY
#define CPU_FREQUENCY         0
#endif

////////////////////////////////////////////////////////////////////////////////
// Module configuration

#if 0 || ( defined( BUILD_LCD ) ) || ( defined( BUILD_RTC ) )
#define PLATFORM_MODULES_LINE            _ROM( "avr32", luaopen_platform, platform_map )
#define PS_LIB_TABLE_NAME                "mizar32"
#define PLATFORM_MODULES_ENABLE
#else
#define PLATFORM_MODULES_LINE
#warning Unable to include platform modules in the image
#endif

#if defined( BUILD_ADC ) && ( NUM_ADC > 0 )
#define MODULE_ADC_LINE                  _ROM( AUXLIB_ADC, luaopen_adc, adc_map )
#else
#define MODULE_ADC_LINE
#warning Unable to include generic module 'adc' in the image
#endif

#if defined( BUILD_NIFFS )
#define MODULE_FS_LINE                   _ROM( AUXLIB_FS, luaopen_fs, fs_map )
#else
#define MODULE_FS_LINE
#warning Unable to include generic module 'fs' in the image
#endif

#define MODULE_LUA_STRING_LINE           _ROM( LUA_STRLIBNAME, luaopen_string, strlib )
#define MODULE_BIT_LINE                  _ROM( AUXLIB_BIT, luaopen_bit, bit_map )

#if ( NUM_TIMER > 0 )
#define MODULE_TMR_LINE                  _ROM( AUXLIB_TMR, luaopen_tmr, tmr_map )
#else
#define MODULE_TMR_LINE
#warning Unable to include generic module 'tmr' in the image
#endif

#if defined( BUILD_TERM )
#define MODULE_TERM_LINE                 _ROM( AUXLIB_TERM, luaopen_term, term_map )
#else
#define MODULE_TERM_LINE
#warning Unable to include generic module 'term' in the image
#endif

#define MODULE_LUA_CO_LINE               _ROM( LUA_COLIBNAME, luaopen_dummy, co_funcs )
#define MODULE_ELUA_LINE                 _ROM( AUXLIB_ELUA, luaopen_elua, elua_map )
#define MODULE_PACK_LINE                 _ROM( AUXLIB_PACK, luaopen_pack, pack_map )

#if ( NUM_I2C > 0 )
#define MODULE_I2C_LINE                  _ROM( AUXLIB_I2C, luaopen_i2c, i2c_map )
#else
#define MODULE_I2C_LINE
#warning Unable to include generic module 'i2c' in the image
#endif

#if defined( BUILD_UIP )
#define MODULE_NET_LINE                  _ROM( AUXLIB_NET, luaopen_net, net_map )
#else
#define MODULE_NET_LINE
#warning Unable to include generic module 'net' in the image
#endif

#if ( NUM_SPI > 0 )
#define MODULE_SPI_LINE                  _ROM( AUXLIB_SPI, luaopen_spi, spi_map )
#else
#define MODULE_SPI_LINE
#warning Unable to include generic module 'spi' in the image
#endif

#if ( NUM_UART > 0 )
#define MODULE_UART_LINE                 _ROM( AUXLIB_UART, luaopen_uart, uart_map )
#else
#define MODULE_UART_LINE
#warning Unable to include generic module 'uart' in the image
#endif

#define MODULE_LUA_DEBUG_LINE            _ROM( LUA_DBLIBNAME, luaopen_debug, dblib )
#define MODULE_LUA_TABLE_LINE            _ROM( LUA_TABLIBNAME, luaopen_table, tab_funcs )
#define MODULE_LUA_MATH_LINE             _ROM( LUA_MATHLIBNAME, luaopen_math, math_map )
#define MODULE_CPU_LINE                  _ROM( AUXLIB_CPU, luaopen_cpu, cpu_map )

#if ( NUM_PWM > 0 )
#define MODULE_PWM_LINE                  _ROM( AUXLIB_PWM, luaopen_pwm, pwm_map )
#else
#define MODULE_PWM_LINE
#warning Unable to include generic module 'pwm' in the image
#endif

#if ( NUM_PIO > 0 )
#define MODULE_PIO_LINE                  _ROM( AUXLIB_PIO, luaopen_pio, pio_map )
#else
#define MODULE_PIO_LINE
#warning Unable to include generic module 'pio' in the image
#endif

#define MODULE_PD_LINE                   _ROM( AUXLIB_PD, luaopen_pd, pd_map )

#define LUA_PLATFORM_LIBS_ROM\
  PLATFORM_MODULES_LINE\
  MODULE_ADC_LINE\
  MODULE_FS_LINE\
  MODULE_LUA_STRING_LINE\
  MODULE_BIT_LINE\
  MODULE_TMR_LINE\
  MODULE_TERM_LINE\
  MODULE_LUA_CO_LINE\
  MODULE_ELUA_LINE\
  MODULE_PACK_LINE\
  MODULE_I2C_LINE\
  MODULE_NET_LINE\
  MODULE_SPI_LINE\
  MODULE_UART_LINE\
  MODULE_LUA_DEBUG_LINE\
  MODULE_LUA_TABLE_LINE\
  MODULE_LUA_MATH_LINE\
  MODULE_CPU_LINE\
  MODULE_PWM_LINE\
  MODULE_PIO_LINE\
  MODULE_PD_LINE

#if defined( BUILD_LCD )
#define PL_MODULE_LCD_LINE               _ROM( "lcd", luaopen_dummy, lcd_map )
#else
#define PL_MODULE_LCD_LINE
#warning Unable to include platform specific module 'lcd' in the image
#endif

#if defined( BUILD_RTC )
#define PL_MODULE_RTC_LINE               _ROM( "rtc", luaopen_dummy, rtc_map )
#else
#define PL_MODULE_RTC_LINE
#warning Unable to include platform specific module 'rtc' in the image
#endif

#define PLATFORM_MODULES_LIBS_ROM\
  PL_MODULE_LCD_LINE\
  PL_MODULE_RTC_LINE

#define LUA_LIBS_NOLTR\
  { LUA_IOLIBNAME, luaopen_io },\
  { LUA_LOADLIBNAME, luaopen_package },\

#endif // #ifndef __GENERATED_MIZAR32A_H__
