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
#include "ADC/adc.h"
#include "I2C/i2c.h"
#include "GPIO/gpio.h"
#include "LCD/LCD.h"
#include "SI5351/Si5351.h"

#include <cr_section_macros.h>
#include <NXP/crp.h>

#include "config.h"
#include <stdio.h>

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

unsigned short samples[128];

void cal_task( void * data)
{
	// initialize LCD
	lcd_clear_all();
	lcd_init();
	lcd_led_power(1);

	// draw splash screen
	lcd_text(32,10,3,"NFC",0);
	lcd_text(12,40,1,"ipTronix   Arrow   NXP",0);
	lcd_update(0,63);

	// initialize pll
	Si5351_Initialize();

	// prepare new screen during splash wait
	lcd_clear_all();
	lcd_vert_line(10,63,64);
	vTaskDelay(500);

	// application main loop
	for (;;)
	{
		int i;
		int max = 0;
		int maxval = 0;
		for (i=0;i<128;i++)
		{
			Si5351_OutputFrequency((9560000+i*8000000/128)*25/24);
			lcd_update(0,63);
			//ADCBurstReadStart();
			vTaskDelay(1);
			samples[i] = ADCRead(3);//ADCValue[3];
			if (i!=64)
				lcd_clear_area(i,10,i,63);
			lcd_vert_line(63-samples[i]*51/1023,63,i);
			if (maxval<samples[i])
			{
				maxval = samples[i];
				max = i;
			}

			if (GPIOGetPinValue( PIN_ISP ) == 0)
			{
				lcd_clear_all();
				lcd_text(52,26,2,"ISP",0);
				lcd_update(0,63);
				NVIC_SystemReset();
			}
		}
		char str[20];
		float freq= 9.56+(float)max*8/128;
		sprintf(str,"fmax = %2d.%03d MHz",(int)freq,((int)(freq*1000))%1000);
		lcd_text(0, 0 , 0, str, 0);
		lcd_update(0,63);
		vTaskDelay(1500);
		lcd_clear_all();
		lcd_vert_line(10,63,64);
	}

}

int main(void) {

	if (GPIOGetPinValue( PIN_ISP ) == 0)
	{
		ReinvokeISP();
	}
	/* Prepare the hardware to run this demo. */
	prvSetupHardware();

	xTaskCreate(cal_task,                               /* The function that implements the task. */
				( signed char * ) "CAL",                /* The text name assigned to the task - for debug only as it is not used by the kernel. */
				200,                                    /* The size of the stack to allocate to the task. */
				NULL,                                   /* The parameter passed to the task - just to check the functionality. */
				tskIDLE_PRIORITY,                       /* The priority assigned to the task. */
				NULL );

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

	// init CLKOUT pin
	LPC_IOCON->PIO0_1=1;        // enable CLKOUT
	LPC_SYSCON->CLKOUTSEL=3;    // select main clock
	LPC_SYSCON->CLKOUTUEN=1;      // update clock source
	LPC_SYSCON->CLKOUTDIV=2;      // divide by 2

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
