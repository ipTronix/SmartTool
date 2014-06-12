//*****************************************************************************
//   +--+
//   | ++----+
//   +-++    |
//     |     |
//   +-+--+  |
//   | +--+--+
//   +----+    Copyright (c) 2010 Code Red Technologies Ltd.
//
// retarget.c - provides stub routines to allow printf/scanf from Redlib C library
//              to carry out I/O over UART
//
// Software License Agreement
//
// The software is owned by Code Red Technologies and/or its suppliers, and is
// protected under applicable copyright laws.  All rights are reserved.  Any
// use in violation of the foregoing restrictions may subject the user to criminal
// sanctions under applicable laws, as well as to civil liability for the breach
// of the terms and conditions of this license.
//
// THIS SOFTWARE IS PROVIDED "AS IS".  NO WARRANTIES, WHETHER EXPRESS, IMPLIED
// OR STATUTORY, INCLUDING, BUT NOT LIMITED TO, IMPLIED WARRANTIES OF
// MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE APPLY TO THIS SOFTWARE.
// USE OF THIS SOFTWARE FOR COMMERCIAL DEVELOPMENT AND/OR EDUCATION IS SUBJECT
// TO A CURRENT END USER LICENSE AGREEMENT (COMMERCIAL OR EDUCATIONAL) WITH
// CODE RED TECHNOLOGIES LTD.
//
//*****************************************************************************

// Include stdio.h to pull in __REDLIB_INTERFACE_VERSION__
#include "FreeRTOS.h"
#include "task.h"
#include "USB_CDC/cdc.h"

#if (__REDLIB_INTERFACE_VERSION__ >= 20000)
// We are using new Redlib_v2 semihosting interface
	#define WRITEFUNC __sys_write
	#define READFUNC __sys_readc
#else
// We are using original Redlib semihosting interface
	#define WRITEFUNC __write
	#define READFUNC __readc
#endif

// Function __write() / __sys_write
//
// Called by bottom level of printf routine within RedLib C library to write
// a character. With the default semihosting stub, this would write the character
// to the debugger console window . But this version writes
// the character to the LPC1768/RDB1768 UART.
int WRITEFUNC (int iFileHandle, char *pcBuffer, int iLength)
{
	return iLength;
}

// Function __readc() / __sys_readc
//
// Called by bottom level of scanf routine within RedLib C library to read
// a character. With the default semihosting stub, this would read the character
// from the debugger console window (which acts as stdin). But this version reads
// the character from the LPC1768/RDB1768 UART.
int READFUNC (void)
{
	return 0;
}

void __sys_appexit(void)
{
	vTaskDelete(xTaskGetCurrentTaskHandle());
}

// this function is called after each malloc call in order to verify if heap
// is clashing with stack. this is because heap and stack share the same
// memory region where stack fills from top to bottom and heap from bottom to top

unsigned  __check_heap_overflow(void * new_end_of_heap)
{
	return 0;
}
