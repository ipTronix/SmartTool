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
#include "TIMER/timer16.h"
#include "TIMER/timer32.h"
#include "LCD/LCD.h"
#include "GPIO/GPIO.h"
#include "CAP_KEY/CAP_KEY.h"
#include <cr_section_macros.h>
#include <NXP/crp.h>

#include "config.h"
#include <stdio.h>

#include "FreeRTOS.h"
#include "semphr.h"

/*-----------------------------------------------------------*/

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
static int32_t nAmount;
static int32_t nCardAmount;

typedef enum
{
	stIDLE,
	stSELECT,
	stVENDING,
	stRECHARGE_SEL,
	stRECHARGING,
	stSHOWCREDIT,
} STATE;

static STATE eState;
static int nCurrentItem;
static char cStateChange;
int nTransactionAmount;

#define ITEM_STR_LEN 20

typedef struct
{
	char acItem[ITEM_STR_LEN];
	int price;
} ITEM;

const ITEM asItemList[] =
{
		{ "Chips", 250},
		{ "Candy bar",100},
		{ "Water",150},
		{ "Chocolate", 300},
		{ "Ice Cream", 350}
};

#define MAX_ITEMS (sizeof(asItemList)/sizeof(ITEM))
portBASE_TYPE qres;

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

void ui_task( void * data)
{
	eState = stIDLE;
	nCurrentItem = 0;

	// init LCD;
	lcd_clear_all();
	lcd_init();
	lcd_led_power(1);
	//lcd_test(0);
	lcd_text(32,10,3,"NFC",0);
	lcd_text(12,40,1,"ipTronix   Arrow   NXP",0);
	lcd_update(0,63);

	lcd_clear_all();
	vTaskDelay(1500);
	cStateChange = 1;

	GPIOSetBitValue(PIN_MOSFET,0);
	for (;;)
	{
		char str[30];
		KEY_EVENT_t key;

		key = CAP_KEY_GetEvent();
		qres = xQueueReceive(xUIQueue,&nCardAmount,100);

		if (GPIOGetPinValue( PIN_ISP ) == 0)
		{
			lcd_clear_all();
			lcd_text(52,26,2,"ISP",0);
			lcd_update(0,63);
			NVIC_SystemReset();
		}

		if (key!=keNONE || cStateChange || qres==pdPASS)
		{
			lcd_clear_all();

			do
			{
				cStateChange = 0;
				switch (eState)
				{
					case stIDLE:
					{
						if (key==keK1_PRESS)
						{
							eState = stSELECT;
							cStateChange = 1;
							nCurrentItem = 0;
						}
						else if (key==keK3_PRESS)
						{
							eState = stRECHARGE_SEL;
							cStateChange = 1;
						}
						else
						{
							lcd_text(32,0,1,"Vending Demo",0);
							lcd_text(12,20,2,"Select function",0);
							lcd_text( 0,52,2,"   Buy   ",1);
							lcd_text(75,52,2," Charge ",1);
						}
					}
					break;
					case stSELECT:
					{
						if (key==keK2_PRESS)
						{
							eState=stVENDING;
							nTransactionAmount = -asItemList[nCurrentItem].price;
							cStateChange=1;
						}
						else
						{
							if ((key==keK1_PRESS || key==keK1_REPEAT)&& nCurrentItem>0)
								nCurrentItem--;
							else if ((key==keK3_PRESS || key==keK3_REPEAT)&& nCurrentItem<(MAX_ITEMS-1))
								nCurrentItem++;
							{
								lcd_text(22,0,2," select item",0);
								lcd_text( 0,52,1,"   /\\   ",1);
								lcd_text(40,52,1,"   Select   ",1);
								lcd_text(100,52,1,"   \\/   ",1);
								for (int i=0;i<3;i++)
								{
									if ((i+nCurrentItem)>0 && (i+nCurrentItem)<=MAX_ITEMS)
									{
										sprintf(str,
												    "%2d.%02d",
														asItemList[i-1+nCurrentItem].price/100,
														asItemList[i-1+nCurrentItem].price%100);
										lcd_text(12, 13*(i+1),2,asItemList[i-1+nCurrentItem].acItem,0);
										lcd_text(90, 13*(i+1),2,str,0);
									}
								}
								lcd_invert_area(0,13*2,127,13*3);
							}
						}
					}
					break;
					case stVENDING:
					{
						if (key==keK1_PRESS)
						{
							eState=stIDLE;
							cStateChange=1;
							nTransactionAmount = 0;
						}
						else if (qres==pdPASS)
						{
							eState=stSHOWCREDIT;
							cStateChange=1;
						}
						else
						{
							lcd_text(32,20,2,"Scan Card",0);
							lcd_text( 0,52,1,"Cancel",1);
						}
					}
					break;
					case stRECHARGE_SEL:
					{
						if (key==keK2_PRESS)
						{
							eState=stRECHARGING;
							cStateChange = 1;
							nTransactionAmount = nAmount;
						}
						else
						{
							if ((key==keK1_PRESS || key==keK1_REPEAT)&& nAmount<1000)
								nAmount+=10;
							else if ((key==keK3_PRESS || key==keK3_REPEAT)&& nAmount>0)
								nAmount-=10;
							sprintf(str,"%2d.%02d",nAmount/100,nAmount%100);
							lcd_text(12,0,2,"Select amount",0);
							lcd_text(12,15,3,str,0);
							lcd_text( 0,52,1,"   /\\   ",1);
							lcd_text(40,52,1,"   Select   ",1);
							lcd_text(100,52,1,"   \\/   ",1);
						}
					}
					break;
					case stRECHARGING:
					{
						if (key==keK1_PRESS)
						{
							eState=stIDLE;
							cStateChange=1;
							nTransactionAmount = 0;
							key = keNONE;
						}
						else if (qres==pdPASS)
						{
							eState=stSHOWCREDIT;
							cStateChange=1;
						}
						else
						{
							lcd_text(32,20,2,"Scan Card",0);
							lcd_text( 0,52,1,"Cancel",1);
						}
					}
					break;
					case stSHOWCREDIT:
					{

						if (nCardAmount <0)
						{
							nCardAmount = -nCardAmount;
							lcd_text(0,0,2,"Not enough Credit:",0);
						}
						else
						{
							lcd_text(12,0,2,"Credit:",0);
						}
						sprintf(str,"%d.%02d",nCardAmount/100,nCardAmount%100);
						lcd_text(12,12,3,str,0);
						lcd_update(0,63);
						vTaskDelay(2000);
						GPIOSetBitValue(PIN_MOSFET,0);
						eState=stIDLE;
						cStateChange=1;
						lcd_clear_all();
					}
					break;
				}
				key = keNONE;
			} while (cStateChange);
			lcd_update(0,63);
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

	nTransactionAmount = 0;

	xTaskCreate(ui_task,                                /* The function that implements the task. */
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
	xUIQueue = xQueueCreate(1,sizeof(int32_t));

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

	// init MOSFET pin
	LPC_IOCON->PIO1_25=0x90;
	GPIOSetDir(PIN_MOSFET,1);

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
