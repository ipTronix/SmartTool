/****************************************************************************
 *   $Id:: ssp.c 4102 2010-08-02 20:53:05Z usb00423                         $
 *   Project: NXP LPC11Uxx SSP example
 *
 *   Description:
 *     This file contains SSP code example which include SSP 
 *     initialization, SSP interrupt handler, and APIs for SSP
 *     reading.
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
* documentation is hereby granted, under NXP Semiconductors' 
* relevant copyright in the software, without fee, provided that it 
* is used in conjunction with NXP Semiconductors microcontrollers.  This 
* copyright, permission, and disclaimer notice must appear in all copies of 
* this code.

****************************************************************************/

#include "LPC11Uxx.h"			/* LPC11xx Peripheral Registers */
#include "GPIO/gpio.h"
#include "SSP/ssp.h"
#include "config.h"

/*****************************************************************************
** Function name:		SSP0_IRQHandler
**
** Descriptions:		SSP port is used for SPI communication.
**						SSP interrupt handler
**						The algorithm is, if RXFIFO is at least half full, 
**						start receive until it's empty; if TXFIFO is at least
**						half empty, start transmit until it's full.
**						This will maximize the use of both FIFOs and performance.
**
** parameters:			None
** Returned value:		None
** 
*****************************************************************************/
void SSP0_IRQHandler(void) 
{
  uint32_t regValue;

  regValue = LPC_SSP0->MIS;
  if ( regValue & SSPMIS_RORMIS )	/* Receive overrun interrupt */
  {
//	interruptOverRunStat0++;
	LPC_SSP0->ICR = SSPICR_RORIC;	/* clear interrupt */
  }
  if ( regValue & SSPMIS_RTMIS )	/* Receive timeout interrupt */
  {
//	interruptRxTimeoutStat0++;
	LPC_SSP0->ICR = SSPICR_RTIC;	/* clear interrupt */
  }

  /* please be aware that, in main and ISR, CurrentRxIndex and CurrentTxIndex
  are shared as global variables. It may create some race condition that main
  and ISR manipulate these variables at the same time. SSPSR_BSY checking (polling)
  in both main and ISR could prevent this kind of race condition */
  if ( regValue & SSPMIS_RXMIS )	/* Rx at least half full */
  {
//	interruptRxStat0++;		/* receive until it's empty */
  }
  return;
}

/*****************************************************************************
** Function name:		SSP1_IRQHandler
**
** Descriptions:		SSP port is used for SPI communication.
**						SSP interrupt handler
**						The algorithm is, if RXFIFO is at least half full, 
**						start receive until it's empty; if TXFIFO is at least
**						half empty, start transmit until it's full.
**						This will maximize the use of both FIFOs and performance.
**
** parameters:			None
** Returned value:		None
** 
*****************************************************************************/
void SSP1_IRQHandler(void) 
{
  uint32_t regValue;

  regValue = LPC_SSP1->MIS;
  if ( regValue & SSPMIS_RORMIS )	/* Receive overrun interrupt */
  {
//	interruptOverRunStat1++;
	LPC_SSP1->ICR = SSPICR_RORIC;	/* clear interrupt */
  }
  if ( regValue & SSPMIS_RTMIS )	/* Receive timeout interrupt */
  {
//	interruptRxTimeoutStat1++;
	LPC_SSP1->ICR = SSPICR_RTIC;	/* clear interrupt */
  }

  /* please be aware that, in main and ISR, CurrentRxIndex and CurrentTxIndex
  are shared as global variables. It may create some race condition that main
  and ISR manipulate these variables at the same time. SSPSR_BSY checking (polling)
  in both main and ISR could prevent this kind of race condition */
  if ( regValue & SSPMIS_RXMIS )	/* Rx at least half full */
  {
//	interruptRxStat1++;		/* receive until it's empty */
  }
  return;
}

/*****************************************************************************
** Function name:		SSP_IOConfig
**
** Descriptions:		SSP port initialization routine
**				
** parameters:			None
** Returned value:		None
** 
*****************************************************************************/
void SSP_IOConfig( uint8_t portNum )
{
  if ( portNum == 0 )
  {
	LPC_SYSCON->PRESETCTRL |= (0x1<<0);
	LPC_SYSCON->SYSAHBCLKCTRL |= (0x1<<11);
	LPC_SYSCON->SSP0CLKDIV = 0x2;			/* Divided by 2 */
	LPC_IOCON->PIO0_8 &= ~0x07;		/*  SSP I/O config */
	LPC_IOCON->PIO0_8 |= 0x01;		/* SSP MISO */
	LPC_IOCON->PIO0_9 &= ~0x07;	
	LPC_IOCON->PIO0_9 |= 0x01;		/* SSP MOSI */

#if 0 /*  Set to 1 if __SWD_DISABLED */ 
	LPC_IOCON->SWCLK_PIO0_10 &= ~0x07;
	LPC_IOCON->SWCLK_PIO0_10 |= 0x02;		/* SSP CLK */
#endif
        
#if SSP0_CLK_P1_29
	/* On 11Uxx , SSP CLK can be routed to different pins. */
	LPC_IOCON->PIO1_29 &= ~0x07;	/* SSP CLK */
	LPC_IOCON->PIO1_29 = 0x01;
#else
	LPC_IOCON->PIO0_6 &= ~0x07;	
	LPC_IOCON->PIO0_6 = 0x02;
#endif


#if USE_CS
	LPC_IOCON->PIO0_2 &= ~0x07;	
	LPC_IOCON->PIO0_2 |= 0x01;		/* SSP SSEL */
#else
	/* Enable AHB clock to the GPIO domain. */
	LPC_SYSCON->SYSAHBCLKCTRL |= (1<<6);

	LPC_IOCON->PIO0_2 &= ~0x07;		/* SSP SSEL is a GPIO pin */
	/* port0, bit 2 is set to GPIO output and high */
	GPIOSetDir( PORT0, 2, 1 );
	GPIOSetBitValue( PORT0, 2, 1 );
#endif
  }
  else		/* port number 1 */
  {
	LPC_SYSCON->PRESETCTRL |= (0x1<<2);
	LPC_SYSCON->SYSAHBCLKCTRL |= (1<<18);
	LPC_SYSCON->SSP1CLKDIV = 0x02;			/* Divided by 2 */
#if SSP1_CLK_P1_20
	LPC_IOCON->PIO1_20 &= ~0x07;    /*  SSP I/O config */
	LPC_IOCON->PIO1_20 |= 0x02;		/* SSP CLK */
#elif SSP1_CLK_P1_15
	LPC_IOCON->PIO1_15 &= ~0x07;    /*  SSP I/O config */
	LPC_IOCON->PIO1_15 |= 0x03;		/* SSP CLK */
#endif

#if SSP1_MISO_P1_21
	LPC_IOCON->PIO1_21 &= ~0x07;	
	LPC_IOCON->PIO1_21 |= 0x02;		/* SSP MISO */
#elif SSP1_MISO_P0_22
	LPC_IOCON->PIO0_22 &= ~0x07;	
	LPC_IOCON->PIO0_22 |= 0x03;		/* SSP MISO */
#endif
#if SSP1_MOSI_P1_22
	LPC_IOCON->PIO1_22 &= ~0x07;	
	LPC_IOCON->PIO1_22 |= 0x02;		/* SSP MOSI */
#elif SSP1_MOSI_P0_21
	LPC_IOCON->PIO0_21 &= ~0x07;	
	LPC_IOCON->PIO0_21 |= 0x02;		/* SSP MOSI */
#endif
 
#if USE_CS
#if SSP1_CS_P1_23
	LPC_IOCON->PIO1_23 &= ~0x07;	
	LPC_IOCON->PIO1_23 |= 0x02;		/* SSP SSEL */
#elsif SSP1_CS_P1_19
	LPC_IOCON->PIO1_19 &= ~0x07;	
	LPC_IOCON->PIO1_19 |= 0x02;		/* SSP SSEL */
#endif
#else
	/* Enable AHB clock to the GPIO domain. */
	LPC_SYSCON->SYSAHBCLKCTRL |= (1<<6);

#if SSP1_CS_P1_23
	LPC_IOCON->PIO1_23 &= ~0x07;		/* SSP SSEL is a GPIO pin */
	/* port2, bit 0 is set to GPIO output and high */
	GPIOSetDir( PORT1, 23, 1 );
	GPIOSetBitValue( PORT1, 23, 1 );
#elif SSP1_CS_P1_19
	LPC_IOCON->PIO1_19 &= ~0x07;		/* SSP SSEL is a GPIO pin */
	/* port2, bit 0 is set to GPIO output and high */
	GPIOSetDir( PORT1, 19, 1 );
	GPIOSetBitValue( PORT1, 19, 1 );
#elif SSP1_CS_P0_13
//	LPC_IOCON->TDO_PIO0_13 &= ~0x07;		/* SSP SSEL is a GPIO pin */
	LPC_IOCON->TDO_PIO0_13 |= 1;		/* SSP SSEL is a GPIO pin */
	/* port2, bit 0 is set to GPIO output and high */
	GPIOSetDir( PORT0, 13, 1 );
	GPIOSetBitValue( PORT0, 13, 1 );
#endif

#endif
  }
  return;		
}

/*****************************************************************************
** Function name:		SSP_Init
**
** Descriptions:		SSP port initialization routine
**				
** parameters:			None
** Returned value:		None
** 
*****************************************************************************/
void SSP_Init( uint8_t portNum )
{
  volatile uint8_t i, Dummy;
  if ( portNum == 0 )
  {
	/* Set DSS data to 8-bit, Frame format SPI, CPOL = 0, CPHA = 0, and SCR is 15 */
	LPC_SSP0->CR0 = 0x0707;

	/* SSPCPSR clock prescale register, master mode, minimum divisor is 0x02 */
	LPC_SSP0->CPSR = 0x2;

	for ( i = 0; i < FIFOSIZE; i++ )
	{
	  Dummy = LPC_SSP0->DR;		/* clear the RxFIFO */
	}

	/* Enable the SSP Interrupt */
	NVIC_EnableIRQ(SSP0_IRQn);
	
	/* Device select as master, SSP Enabled */
#if LOOPBACK_MODE
	LPC_SSP0->CR1 = SSPCR1_LBM | SSPCR1_SSE;
#else
#if SSP_SLAVE
	/* Slave mode */
	if ( LPC_SSP0->CR1 & SSPCR1_SSE )
	{
	  /* The slave bit can't be set until SSE bit is zero. */
	  LPC_SSP0->CR1 &= ~SSPCR1_SSE;
	}
	LPC_SSP0->CR1 = SSPCR1_MS;		/* Enable slave bit first */
	LPC_SSP0->CR1 |= SSPCR1_SSE;	/* Enable SSP */
#else
	/* Master mode */
	LPC_SSP0->CR1 = SSPCR1_SSE;
#endif
#endif
	/* Set SSPINMS registers to enable interrupts */
	/* enable all error related interrupts */
	LPC_SSP0->IMSC = SSPIMSC_RORIM | SSPIMSC_RTIM;
  }
  else
  {
	/* Set DSS data to 8-bit, Frame format SPI, CPOL = 0, CPHA = 0, and SCR is 15 */
	LPC_SSP1->CR0 = 0x0707;

	/* SSPCPSR clock prescale register, master mode, minimum divisor is 0x02 */
	LPC_SSP1->CPSR = 0x2;

	for ( i = 0; i < FIFOSIZE; i++ )
	{
	  Dummy = LPC_SSP1->DR;		/* clear the RxFIFO */
	}

	/* Enable the SSP Interrupt */
	NVIC_EnableIRQ(SSP1_IRQn);
	
	/* Device select as master, SSP Enabled */
#if LOOPBACK_MODE
	LPC_SSP1->CR1 = SSPCR1_LBM | SSPCR1_SSE;
#else
#if SSP_SLAVE
	/* Slave mode */
	if ( LPC_SSP1->CR1 & SSPCR1_SSE )
	{
	  /* The slave bit can't be set until SSE bit is zero. */
	  LPC_SSP1->CR1 &= ~SSPCR1_SSE;
	}
	LPC_SSP1->CR1 = SSPCR1_MS;		/* Enable slave bit first */
	LPC_SSP1->CR1 |= SSPCR1_SSE;	/* Enable SSP */
#else
	/* Master mode */
	LPC_SSP1->CR1 = SSPCR1_SSE;
#endif
#endif
	/* Set SSPINMS registers to enable interrupts */
	/* enable all error related interrupts */
	LPC_SSP1->IMSC = SSPIMSC_RORIM | SSPIMSC_RTIM;
  }
  return;
}

/**
 * @brief Perform bit reversal on one byte \a x
 *
 * @return reversed byte
 */

static uint8_t
bit_reversal(const uint8_t x)
{
  uint8_t ret = x;
  ret = (((ret & 0xaa) >> 1) | ((ret & 0x55) << 1));
  ret = (((ret & 0xcc) >> 2) | ((ret & 0x33) << 2));
  ret = (((ret & 0xf0) >> 4) | ((ret & 0x0f) << 4));
  return ret;
}

void SSP_SetCS(uint8_t portNum, uint8_t state)
{
#if	USE_CS==0
	state = 1-state;
  if (portNum==0)
  {
	GPIOSetBitValue( PORT0, 2, state );
  }
  else
  {

#if SSP1_CS_P1_23
	GPIOSetBitValue( PORT1, 23, state );
#elif SSP1_CS_P1_19
	GPIOSetBitValue( PORT1, 19, state );
#elif SSP1_CS_P0_13
	GPIOSetBitValue( PORT0, 13, state );
#endif
  }
#endif
}

void SSP_Send( uint8_t portNum, uint8_t *buf, uint32_t Length, uint8_t lsb_first )
{
  SSP_SetCS(portNum,1);
  SSP_Send_naked( portNum, buf, Length, lsb_first );
  SSP_SetCS(portNum,0);
}

/*****************************************************************************
** Function name:		SSP_Send
**
** Descriptions:		Send a block of data to the SSP port, the 
**						first parameter is the buffer pointer, the 2nd 
**						parameter is the block length.
**
** parameters:			port #, buffer pointer, and the block length
** Returned value:		None
** 
*****************************************************************************/
void SSP_Send_naked( uint8_t portNum, uint8_t *buf, uint32_t Length, uint8_t lsb_first )
{
  uint32_t i;
  volatile uint8_t Dummy;
//  LOG_HEX("SSP","TX",buf,Length);
  for ( i = 0; i < Length; i++ )
  {
	if ( portNum == 0 )
	{
	  /* Move on only if NOT busy and TX FIFO not full. */
	  while ( (LPC_SSP0->SR & (SSPSR_TNF|SSPSR_BSY)) != SSPSR_TNF );
	  LPC_SSP0->DR = lsb_first ? bit_reversal(*buf) : *buf;
	  buf++;
#if !LOOPBACK_MODE
	  while ( (LPC_SSP0->SR & (SSPSR_BSY|SSPSR_RNE)) != SSPSR_RNE );
	  /* Whenever a byte is written, MISO FIFO counter increments, Clear FIFO 
	  on MISO. Otherwise, when SSP0Receive() is called, previous data byte
	  is left in the FIFO. */
	  Dummy = LPC_SSP0->DR;
#else
	  /* Wait until the Busy bit is cleared. */
	  while ( LPC_SSP0->SR & SSPSR_BSY );
#endif
	}
	else
	{
	  /* Move on only if NOT busy and TX FIFO not full. */
	  while ( (LPC_SSP1->SR & (SSPSR_TNF|SSPSR_BSY)) != SSPSR_TNF );
	  LPC_SSP1->DR = lsb_first ? bit_reversal(*buf) : *buf;
	  buf++;
#if !LOOPBACK_MODE
	  while ( (LPC_SSP1->SR & (SSPSR_BSY|SSPSR_RNE)) != SSPSR_RNE );
	  /* Whenever a byte is written, MISO FIFO counter increments, Clear FIFO 
	  on MISO. Otherwise, when SSP0Receive() is called, previous data byte
	  is left in the FIFO. */
	  Dummy = LPC_SSP1->DR;
#else
	  /* Wait until the Busy bit is cleared. */
	  while ( LPC_SSP1->SR & SSPSR_BSY );
#endif
	}
  }
  return; 
}

/*****************************************************************************
** Function name:		SSP_Receive
** Descriptions:		the module will receive a block of data from 
**						the SSP, the 2nd parameter is the block 
**						length.
** parameters:			port #, buffer pointer, and block length
** Returned value:		None
** 
*****************************************************************************/
void SSP_Receive_naked( uint8_t portNum, uint8_t *buf, uint32_t Length, uint8_t lsb_first )
{
  uint32_t i;
  uint8_t *ptr=buf;
  uint8_t dummy;
  for ( i = 0; i < Length; i++ )
  {
	/* As long as Receive FIFO is not empty, I can always receive. */
	/* If it's a loopback test, clock is shared for both TX and RX,
	no need to write dummy byte to get clock to get the data */
	/* if it's a peer-to-peer communication, SSPDR needs to be written
	before a read can take place. */
	if ( portNum == 0 )
	{
#if !LOOPBACK_MODE
#if SSP_SLAVE
	  while ( !(LPC_SSP0->SR & SSPSR_RNE) );
#else
	  LPC_SSP0->DR = 0xff;
	  /* Wait until the Busy bit is cleared */
	  while ( (LPC_SSP0->SR & (SSPSR_BSY|SSPSR_RNE)) != SSPSR_RNE );
#endif
#else
	  while ( !(LPC_SSP0->SR & SSPSR_RNE) );
#endif
	  if (ptr)
	  {
		  *ptr = LPC_SSP0->DR;
		  if (lsb_first) *ptr = bit_reversal(*ptr);
		  ptr++;
	  }
	  else dummy = LPC_SSP0->DR;
	}
	else
	{
#if !LOOPBACK_MODE
#if SSP_SLAVE
	  while ( !(LPC_SSP1->SR & SSPSR_RNE) );
#else
	  LPC_SSP1->DR = 0xff;
	  /* Wait until the Busy bit is cleared */
	  while ( (LPC_SSP1->SR & (SSPSR_BSY|SSPSR_RNE)) != SSPSR_RNE );
#endif
#else
	  while ( !(LPC_SSP1->SR & SSPSR_RNE) );
#endif
	  if (ptr)
	  {
		  *ptr = LPC_SSP1->DR;
          if (lsb_first) *ptr = bit_reversal(*ptr);
          ptr++;
	  }
	  else dummy = LPC_SSP0->DR;
	}
  }
//  LOG_HEX("SSP","RX",buf,Length);

  return; 
}

void SSP_Receive( uint8_t portNum, uint8_t *buf, uint32_t Length, uint8_t lsb_first )
{
	SSP_SetCS(portNum,1);
	SSP_Receive_naked( portNum, buf, Length, lsb_first );
	SSP_SetCS(portNum,0);
}

/*****************************************************************************
** Function name:               SSP_Send
**
** Descriptions:                Send a block of data to the SSP port, the
**                                              first parameter is the buffer pointer, the 2nd
**                                              parameter is the block length.
**
** parameters:                  port #, buffer pointer, and the block length
** Returned value:              None
**
*****************************************************************************/
void SSP_Send_Receive(uint8_t portNum, const uint8_t *pbtTx, const size_t szTx, uint8_t *pbtRx, const size_t szRx, uint8_t lsb_first)
{
  SSP_Send( portNum, pbtTx, szTx, lsb_first );
  SSP_Receive(  portNum, pbtRx, szRx, lsb_first );
}
/******************************************************************************
**                            End Of File
******************************************************************************/

