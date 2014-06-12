/****************************************************************************
 *   $Id:: uart.c 9188 2012-02-16 20:53:43Z nxp41306                        $
 *   Project: NXP LPC11Uxx UART example
 *
 *   Description:
 *     This file contains UART code example which include UART 
 *     initialization, UART interrupt handler, and related APIs for 
 *     UART access.
 *
****************************************************************************
* Software that is described herein is for illustrative purposes only
* which provides customers with programming information regarding the
* products. This software is supplied "AS IS" without any warranties.
* NXP Semiconductors assumes no responsibility or liability for the
* use of the software, conveys no license or title under any patent,
* copyright, or mask work right to the product. NXP Semiconductors
* reserves the right to make changes in the software without
* notification. NXP Semiconductors also make no representation or
* warranty that such application will be suitable for the specified
* use without further testing or modification.

* Permission to use, copy, modify, and distribute this software and its 
* documentation is hereby granted, under NXP Semiconductors� 
* relevant copyright in the software, without fee, provided that it 
* is used in conjunction with NXP Semiconductors microcontrollers.  This 
* copyright, permission, and disclaimer notice must appear in all copies of 
* this code.

****************************************************************************/

#include "LPC11Uxx.h"
#include "stddef.h"
#include "stdbool.h"
#include "uart.h"
#include "semphr.h"
#include <cr_section_macros.h>
#include "config.h"

volatile uint32_t UARTStatus;
volatile uint8_t  UARTTxEmpty = 1;
volatile uint32_t UARTCount = 0;
static xQueueHandle txq;
static xQueueHandle rxq;
static xSemaphoreHandle   txs;
static xSemaphoreHandle   rxs;

#if AUTOBAUD_ENABLE
volatile uint32_t UARTAutoBaud = 0, AutoBaudTimeout = 0;
#endif

/*****************************************************************************
** Function name:		UART_IRQHandler
**
** Descriptions:		UART interrupt handler
**
** parameters:			None
** Returned value:		None
** 
*****************************************************************************/
void UART_IRQHandler(void)
{
  uint8_t IIRValue, LSRValue;
  uint8_t Dummy = Dummy;

  IIRValue = LPC_USART->IIR;
    
  IIRValue >>= 1;			/* skip pending bit in IIR */
  IIRValue &= 0x07;			/* check bit 1~3, interrupt identification */
  if (IIRValue == IIR_RLS)		/* Receive Line Status */
  {
    LSRValue = LPC_USART->LSR;
    /* Receive Line Status */
    if (LSRValue & (LSR_OE | LSR_PE | LSR_FE | LSR_RXFE | LSR_BI))
    {
      /* There are errors or break interrupt */
      /* Read LSR will clear the interrupt */
      UARTStatus = LSRValue;
      Dummy = LPC_USART->RBR;	/* Dummy read on RX to clear 
								interrupt, then bail out */
      return;
    }
    if (LSRValue & LSR_RDR)	/* Receive Data Ready */			
    {
      /* If no error on RLS, normal ready, save into the data buffer. */
      /* Note: read RBR will clear the interrupt */
    	xQueueSendFromISR(rxq, &LPC_USART->RBR, pdFALSE);
    }
  }
  else if (IIRValue == IIR_RDA)	/* Receive Data Available */
  {
    /* Receive Data Available */
  	xQueueSendFromISR(rxq, &LPC_USART->RBR, pdFALSE);
  }
  else if (IIRValue == IIR_CTI)	/* Character timeout indicator */
  {
    /* Character Time-out indicator */
    UARTStatus |= 0x100;		/* Bit 9 as the CTI error */
  }
  else if (IIRValue == IIR_THRE)	/* THRE, transmit holding register empty */
  {
  	uint32_t count = 0;
    /* THRE interrupt */
    LSRValue = LPC_USART->LSR;		/* Check status in the LSR to see if
								valid data in U0THR or not */
    //if (LSRValue & LSR_THRE)
    while (uxQueueMessagesWaitingFromISR(txq) && count<16)
    {
    	char c;

    	xQueueReceiveFromISR(txq,&c,NULL);
    	LPC_USART->THR = c;
    	count++;
    }
  }
  return;
}

/***********************************************************************
 *
 * Function: uart_set_divisors
 *
 * Purpose: Determines best dividers to get a target clock rate
 *
 * Processing:
 *     See function.
 *
 * Parameters:
 *     UARTClk    : UART clock
 *     baudrate   : Desired UART baud rate
 *
 * Outputs:
 *	  baudrate : Sets the estimated buadrate value in DLL, DLM, and FDR.
 *
 * Returns: Error status.
 *
 * Notes: None
 *
 **********************************************************************/
uint32_t uart_set_divisors(uint32_t UARTClk, uint32_t baudrate)
{
  uint32_t uClk;
  uint32_t calcBaudrate = 0;
  uint32_t temp = 0;

  uint32_t mulFracDiv, dividerAddFracDiv;
  uint32_t diviser = 0 ;
  uint32_t mulFracDivOptimal = 1;
  uint32_t dividerAddOptimal = 0;
  uint32_t diviserOptimal = 0;

  uint32_t relativeError = 0;
  uint32_t relativeOptimalError = 100000;

  /* get UART block clock */
  uClk = UARTClk >> 4; /* div by 16 */
  /* In the Uart IP block, baud rate is calculated using FDR and DLL-DLM registers
   * The formula is :
   * BaudRate= uClk * (mulFracDiv/(mulFracDiv+dividerAddFracDiv) / (16 * (DLL)
   * It involves floating point calculations. That's the reason the formulae are adjusted with
   * Multiply and divide method.*/
  /* The value of mulFracDiv and dividerAddFracDiv should comply to the following expressions:
   * 0 < mulFracDiv <= 15, 0 <= dividerAddFracDiv <= 15 */
  for (mulFracDiv = 1; mulFracDiv <= 15; mulFracDiv++)
  {
    for (dividerAddFracDiv = 0; dividerAddFracDiv <= 15; dividerAddFracDiv++)
    {
      temp = (mulFracDiv * uClk) / ((mulFracDiv + dividerAddFracDiv));
      diviser = temp / baudrate;
      if ((temp % baudrate) > (baudrate / 2))
        diviser++;

      if (diviser > 2 && diviser < 65536)
      {
        calcBaudrate = temp / diviser;

        if (calcBaudrate <= baudrate)
          relativeError = baudrate - calcBaudrate;
        else
          relativeError = calcBaudrate - baudrate;

        if ((relativeError < relativeOptimalError))
        {
          mulFracDivOptimal = mulFracDiv ;
          dividerAddOptimal = dividerAddFracDiv;
          diviserOptimal = diviser;
          relativeOptimalError = relativeError;
          if (relativeError == 0)
            break;
        }
      } /* End of if */
    } /* end of inner for loop */
    if (relativeError == 0)
      break;
  } /* end of outer for loop  */

  if (relativeOptimalError < (baudrate / 30))
  {
    /* Set the `Divisor Latch Access Bit` and enable so the DLL/DLM access*/
    /* Initialise the `Divisor latch LSB` and `Divisor latch MSB` registers */
    LPC_USART->DLM = (diviserOptimal >> 8) & 0xFF;
    LPC_USART->DLL = diviserOptimal & 0xFF;

    /* Initialise the Fractional Divider Register */
    LPC_USART->FDR = ((mulFracDivOptimal & 0xF) << 4) | (dividerAddOptimal & 0xF);
    return( true );
  }
  return ( false );
}

/*****************************************************************************
** Function name:		UARTInit
**
** Descriptions:		Initialize UART0 port, setup pin select,
**				clock, parity, stop bits, FIFO, etc.
**
** parameters:			UART baudrate
** Returned value:		None
** 
*****************************************************************************/
void UARTInit(uint32_t baudrate)
{
  uint32_t Fdiv;
  uint32_t regVal;

  UARTTxEmpty = 1;
  UARTCount = 0;
  
  NVIC_DisableIRQ(UART_IRQn);
  /* Select only one location from below. */
  LPC_IOCON->PIO0_18 &= ~0x07;    /*  UART I/O config */
  LPC_IOCON->PIO0_18 |= 0x01;     /* UART RXD */
  LPC_IOCON->PIO0_19 &= ~0x07;	
  LPC_IOCON->PIO0_19 |= 0x01;     /* UART TXD */

  /* Enable UART clock */
  LPC_SYSCON->SYSAHBCLKCTRL |= (1<<12);
  LPC_SYSCON->UARTCLKDIV = 0x1;     /* divided by 1 */

  LPC_USART->LCR = 0x83;            /* 8 bits, no Parity, 1 Stop bit */
  Fdiv = ((SystemCoreClock/LPC_SYSCON->UARTCLKDIV)/16)/baudrate ;	/*baud rate */
  LPC_USART->DLM = Fdiv / 256;
  LPC_USART->DLL = Fdiv % 256;
  LPC_USART->FDR = 0x10;		/* Default */

  LPC_USART->LCR = 0x03;		/* DLAB = 0 */
  LPC_USART->FCR = 0x07;		/* Enable and reset TX and RX FIFO. */

  /* Read to clear the line status. */
  regVal = LPC_USART->LSR;

  /* Ensure a clean start, no data in either TX or RX FIFO. */
  while (( LPC_USART->LSR & (LSR_THRE|LSR_TEMT)) != (LSR_THRE|LSR_TEMT) );
  while ( LPC_USART->LSR & LSR_RDR )
  {
	regVal = LPC_USART->RBR;	/* Dump data from RX FIFO */
  }
 
  txq = xQueueCreate(UART_BUFSIZE,1);
  rxq = xQueueCreate(UART_BUFSIZE,1);
  vSemaphoreCreateBinary(txs);
  vSemaphoreCreateBinary(rxs);
  /* Enable the UART Interrupt */
  NVIC_EnableIRQ(UART_IRQn);

  LPC_USART->IER = IER_RBR | IER_THRE | IER_RLS;	/* Enable UART interrupt */
  return;
}

/*****************************************************************************
** Function name:		UARTSend
**
** Descriptions:		Send a block of data to the UART 0 port based
**				on the data length
**
** parameters:		buffer pointer, and data length
** Returned value:	None
** 
*****************************************************************************/
void UARTSend(uint8_t *BufferPtr, uint32_t Length)
{
  
	uint32_t txb= 0;
	xSemaphoreTake(txs,portMAX_DELAY);
	while (!(LPC_USART->LSR & LSR_THRE));
	while (Length>0 && txb<16)
	{
		LPC_USART->THR = *BufferPtr;
		BufferPtr++;
		Length--;
		txb++;
	}

	while (Length)
	{
		xQueueSend(txq,BufferPtr++,portMAX_DELAY);
		Length--;
	}
	xSemaphoreGive(txs);
  return;
}

/*****************************************************************************
** Function name:		UARTReceive
**
** Descriptions:		Send a block of data to the UART 0 port based
**				on the data length
**
** parameters:		buffer pointer, data length and max ticks to wait
** Returned value:	number of characters read
** 
*****************************************************************************/
uint32_t UARTReceive(uint8_t *BufferPtr, uint32_t Length, portTickType delay)
{
	uint32_t rcv=0;
	if (xSemaphoreTake(rxs, delay))
	{
		while (rcv<Length)
		{
			portBASE_TYPE ret = xQueueReceive(rxq,BufferPtr++,delay);
			if (ret!=pdPASS)
				break;
			rcv++;
		}
		xSemaphoreGive(rxs);
	}
  return rcv;
}

/******************************************************************************
**                            End Of File
******************************************************************************/
