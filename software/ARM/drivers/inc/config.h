/*
 * config.h
 *
 *  Created on: 27 Jun 2013
 *      Author: Dario Pennisi
 */

#ifndef CONFIG_H_
#define CONFIG_H_


//libfnc defines
#define PACKAGE_VERSION 0
#define DRIVER_PN532_SPI_ENABLED
#define NFC_STATIC_STRUCTS
//#define LOG
#define PRIuPTR "08x"
#define PRIu32  "08x"
#define PRINTF_MAX_DELAY 1
#define NFC_POLL_TIMEOUT 1000

// libfreefare specific defines
#define LITTLE_ENDIAN 1234
#define BIG_ENDIAN 4321
#define bswap_16(x) __REV16(x)
#define CFSwapInt32LittleToHost(x) x
#define CFSwapInt32HostToLittle(x) x
#define CFSwapInt32BigToHost(x) __REV(x)
#define CFSwapInt32HostToBig(x) __REV(x)
#define BYTE_ORDER LITTLE_ENDIAN

#define CAPKEY_QSIZE 3
#define UART_BUFSIZE         0x40

// hardware specific defines
#define LCD_SPI_PORT 1
#define PN532_SPI_PORT 0

// SSP configuration
#define USE_CS 0
#define SSP0_CLK_P1_29 1

// set spi1 clk to pin P1_15 for LCD
//#define SSP1_CLK_P1_20 1
#define SSP1_CLK_P1_15 1

// MISO is not used
//#define SSP1_MISO_P1_21 1
//#define SSP1_MISO_P0_22 1

// set spi1 MISO to pin P0_21 for LCD
//#define SSP1_MOSI_P1_22 1
#define SSP1_MOSI_P0_21 1

// set spi1 CS to pin P1_19 for LCD
//#define SSP1_CS_P1_23 1
#ifdef HW_REV1
#define SSP1_CS_P1_19 1
#else
#define SSP1_CS_P0_13 1
#endif

// PIN definition
#define PIN_KEY_COMMON  PORT0, 20
#define PIN_LCD_CD		PORT1, 7
#define PIN_LCD_RESET	PORT1, 6
#define PIN_LCD_LED		PORT0, 11
#define PIN_NFC_IRQ		PORT0,16
#define PIN_MOSFET		PORT1,25
#define PIN_CAL_DET		PORT1,27
#define PIN_ISP			PORT0,7

#endif /* CONFIG_H_ */
