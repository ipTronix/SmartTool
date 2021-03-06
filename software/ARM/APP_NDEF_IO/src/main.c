/*
===============================================================================
 Name        : main.c
 Author      :
 Version     :
 Copyright   : Copyright (C)
 Description : main definition
===============================================================================
*/

/* Standard includes. */
#include "string.h"

/* FreeRTOS includes. */
#include "FreeRTOS.h"
#include "task.h"

/* Hardware specific includes. */
#include "lpc11Uxx.h"
#include "SSP/ssp.h"
#include "USB_CDC/cdc.h"
#include "ADC/adc.h"
#include "I2C/i2c.h"
#include "GPIO/gpio.h"
#include "LCD/LCD.h"
#include <cr_section_macros.h>
#include <NXP/crp.h>

#include "config.h"
#include <stdio.h>

#include "FreeRTOS.h"
#include "semphr.h"
#include "taginfo.h"
#include "CAP_KEY/CAP_KEY.h"
#include "app_ndef_define.h"

/*-----------------------------------------------------------*/
unsigned long command[5];
unsigned long result[4];

#define init_msdstate() *((uint32_t *)(0x10000054)) = 0x0
#define IAP_LOCATION 0x1fff1ff1
typedef void (*IAP)(unsigned long[],unsigned long[]);
IAP iap_entry = (IAP) IAP_LOCATION;

void ReinvokeISP(void)
{
  /* make sure USB clock is turned on before calling ISP */
  LPC_SYSCON->SYSAHBCLKCTRL |= 0x04000;
  /* make sure 32-bit Timer 1 is turned on before calling ISP */
  LPC_SYSCON->SYSAHBCLKCTRL |= 0x00400;
  /* make sure GPIO clock is turned on before calling ISP */
  LPC_SYSCON->SYSAHBCLKCTRL |= 0x00040;
  /* make sure IO configuration clock is turned on before calling ISP */
  LPC_SYSCON->SYSAHBCLKCTRL |= 0x10000;

  /* make sure AHB clock divider is 1:1 */
  LPC_SYSCON->SYSAHBCLKDIV = 1;

  /* Send Reinvoke ISP command to ISP entry point*/
  command[0] = 57;

  init_msdstate();                                                            /* Initialize Storage state machine */
  /* Set stack pointer to ROM value (reset default) This must be the last
     piece of code executed before calling ISP, because most C expressions
     and function returns will fail after the stack pointer is changed. */
  __set_MSP(*((uint32_t *)0x00000000));

  /* Enter ISP. We call "iap_entry" to enter ISP because the ISP entry is done
     through the same command interface as IAP. */
  iap_entry(command, result);
  // Not supposed to come back!
}

/*
 * Perform any application specific hardware configuration.  The clocks,
 * memory, etc. are configured before main() is called.
 */
static void prvSetupHardware( void );

// Variable to store CRP value in. Will be placed automatically
// by the linker when "Enable Code Read Protect" selected.
// See crp.h header for more information
__CRP const unsigned int CRP_WORD = CRP_NO_ISP ;

xQueueHandle xUIQueue = NULL;
static taginfo_t taginfo;
eMODE g_eMode;
KEY_EVENT_t key;

void ui_task( void * data)
{
	portBASE_TYPE res;

	// init LCD;
	g_eMode = modeREAD_CARD;
	lcd_clear_all();
	lcd_init();
	lcd_led_power(1);
	//lcd_test(0);
	lcd_text(32,10,3,"NFC",0);
	lcd_text(12,40,1,"ipTronix   Arrow   NXP",0);
	lcd_update(0,63);

	lcd_clear_all();
	vTaskDelay(1500);

	for (;;)
	{
		KEY_EVENT_t key;
		char str[30];
		lcd_clear_all();
		if (g_eMode == modeEMULATE_CARD)
		{
			lcd_text(10,16,1,"Please approach phone",0);
		}
		else
		{
			lcd_text(12,16,1,"Please approach card",0);
		}

		switch (g_eMode)
		{
			case modeREAD_CARD:
				lcd_text(28,40,2,"Read Card",1);
				break;
			case modeWRITE_CARD:
				lcd_text(28,40,2,"Write Card",1);
				break;
			case modeEMULATE_CARD:
				lcd_text(22,40,2,"Emulate Card",1);
				break;
		}
		lcd_update(0,63);

		do
		{
			res = xQueueReceive(xUIQueue,&taginfo,/*portMAX_DELAY*/100);
			key=CAP_KEY_GetEvent();

			if (GPIOGetPinValue( PIN_ISP ) == 0)
			{
				lcd_clear_all();
				lcd_text(52,26,2,"ISP",0);
				lcd_update(0,63);
				NVIC_SystemReset();
			}
		} while ((key==keNONE) && (res!= pdPASS));

		if (key==keK1_PRESS || key==keK1_REPEAT)
		{
			if (g_eMode>modeREAD_CARD)
				g_eMode--;
			else
				g_eMode=modeEMULATE_CARD;
		}
		else if (key==keK3_PRESS || key==keK3_REPEAT)
		{
			if (g_eMode<modeEMULATE_CARD)
				g_eMode++;
			else
				g_eMode=modeREAD_CARD;
		}
		key=keNONE;

		if (res == pdPASS)
		{
			lcd_clear_all();
			if (g_eMode==modeREAD_CARD)
			{
				sprintf(str, "UID ");
				for (int i=0;i<taginfo.szUidLen;i++)
				{
					sprintf(&str[4+i*2],"%02X",taginfo.abtUid[i]);
				}
				lcd_text(0,0,1,str,0);
				sprintf(str,"NDEF Len %d",taginfo.msgLen);
				lcd_text(0,12,1,str,0);
				snprintf(str,30,"MSG %s",taginfo.msg);
				lcd_text(0,24,1,str,0);
			}
			else if (g_eMode==modeWRITE_CARD)
			{
				sprintf(str,"Write card ok");
				lcd_text(10,24,1,str,0);
			}
			else
			{
				if (taginfo.msgLen)
				{
					sprintf(str,"NDEF Len %d",taginfo.msgLen);
					lcd_text(0,12,1,str,0);
					snprintf(str,30,"MSG %s",&taginfo.msg[2]);
					lcd_text(0,24,1,str,0);
				}
				else
				{
					sprintf(str,"Data read");
					lcd_text(0,12,1,str,0);
				}
			}
			lcd_update(0,63);
			vTaskDelay(1500);
		}
	}
}

void nfc_task( void * data);

int main(void) {

	if (GPIOGetPinValue( PIN_ISP ) == 0)
	{
		ReinvokeISP();
	}
	/* Prepare the hardware to run this demo. */
	prvSetupHardware();

	xTaskCreate(ui_task,                          /* The function that implements the task. */
				( signed char * ) "UI",                 /* The text name assigned to the task - for debug only as it is not used by the kernel. */
				200,                                    /* The size of the stack to allocate to the task. */
				NULL,                                   /* The parameter passed to the task - just to check the functionality. */
				tskIDLE_PRIORITY,                       /* The priority assigned to the task. */
				NULL );


	xTaskCreate(nfc_task,                                /* The function that implements the task. */
				( signed char * ) "NFC",                /* The text name assigned to the task - for debug only as it is not used by the kernel. */
				500,                                    /* The size of the stack to allocate to the task. */
				NULL,                                   /* The parameter passed to the task - just to check the functionality. */
				tskIDLE_PRIORITY,                       /* The priority assigned to the task. */
				NULL );

	xUIQueue = xQueueCreate(1,sizeof(taginfo_t));

	/* Start the tasks and timer running. */
	vTaskStartScheduler();

	/* If all is well, the scheduler will now be running, and the following
	line will never be reached.  If the following line does execute, then
	there was insufficient FreeRTOS heap memory available for the idle and/or
	timer tasks	to be created.  See the memory management section on the
	FreeRTOS web site for more details. */
	for( ;; );
}

static void prvSetupHardware( void )
{


	SystemCoreClockUpdate();

    // enable clock to RAM1
	LPC_SYSCON->SYSAHBCLKCTRL |= (1<<26);

	// init GPIO
	GPIOInit();

	// init I2C
	I2CInit(I2CMASTER);

	//init ADC
    ADCInit(ADC_CLK);

	// init SPI ports
	SSP_IOConfig( 0 );
	SSP_Init( 0 );
	SSP_IOConfig( 1 );
	SSP_Init( 1 );

	// init keyboard
	CAP_KEY_Init();

}
/*-----------------------------------------------------------*/

void vApplicationMallocFailedHook( void )
{
	/* vApplicationMallocFailedHook() will only be called if
	configUSE_MALLOC_FAILED_HOOK is set to 1 in FreeRTOSConfig.h.  It is a hook
	function that will get called if a call to pvPortMalloc() fails.
	pvPortMalloc() is called internally by the kernel whenever a task, queue,
	timer or semaphore is created.  It is also called by various parts of the
	demo application.  If heap_1.c or heap_2.c are used, then the size of the
	heap available to pvPortMalloc() is defined by configTOTAL_HEAP_SIZE in
	FreeRTOSConfig.h, and the xPortGetFreeHeapSize() API function can be used
	to query the size of free heap space that remains (although it does not
	provide information on how the remaining heap might be fragmented). */
	taskDISABLE_INTERRUPTS();
	for( ;; );
}
/*-----------------------------------------------------------*/

void vApplicationStackOverflowHook( xTaskHandle pxTask, signed char *pcTaskName )
{
	( void ) pcTaskName;
	( void ) pxTask;

	/* Run time stack overflow checking is performed if
	configCHECK_FOR_STACK_OVERFLOW is defined to 1 or 2.  This hook
	function is called if a stack overflow is detected. */
	taskDISABLE_INTERRUPTS();
	for( ;; );
}
/*-----------------------------------------------------------*/

void vApplicationIdleHook( void )
{
	__WFI();
}
