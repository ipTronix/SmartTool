/*
 * CAP_KEY.c
 *
 *  Created on: 13 Sep 2013
 *      Author: dario
 */
#include "LPC11Uxx.h"			/* LPC11xx Peripheral Registers */
#include "config.h"
#include "CAP_KEY.h"
#include "freeRTOS.h"
#include "queue.h"

#define DEF_DELTA 200
#define KEY_DN_CNT 10
#define KEY_WAIT_CNT 200
#define KEY_RPT_CNT 400
#define KEY_INC_CNT 10
#define KEY_MAX_ACC 10
#define MAX_KEYS 3

int meas[MAX_KEYS];
int fmeas[MAX_KEYS];
int count[MAX_KEYS];
int base[MAX_KEYS];
short cnt[MAX_KEYS];
static char acc[MAX_KEYS];
char keystate;
static xQueueHandle eventq;
char m_cPollEnable;
/*
 * 	count[0]=LPC_CT16B0->CR0;
 count[1]=LPC_CT16B0->CR2;
 count[2]=LPC_CT16B1->CR0;
 maxdelta = 0;
 for (int i= 0; i<2;i++)
 {
 if ((count[i]-base[i])>maxdelta)
 maxdelta = count[i]-base[i];
 }
 if (maxdelta >50)
 threshold = maxdelta*8/10;
 else threshold = 50;
 for (int i= 0; i<2;i++)
 {
 if (base[i]==0)
 base[i]=count[i];
 if ((count[i]-base[i])>threshold && cnt[i]<5)
 cnt[i]++;
 else if (cnt[i]) cnt[i]--;
 if (cnt[i]>3)
 {
 press[i] = 1;
 }
 else press[i]=0;
 if (abs(count[i]-(int)base[i])<(threshold/5) || ((count[i]-((int)base[i]))<0))
 base[i] = base[i]*0.99+count[i]*0.01;
 sprintf(str,"%4d %4d %4d %d",count[i],(int)base[i],(int)(count[i]-base[i]), press[i]);
 lcd_text(12,20*i,1,str,0);
 }
 */
void TIMER16_1_IRQHandler(void)
{
	int maxdelta;
	int threshold;
	KEY_EVENT_t event;
	if (LPC_CT16B1 ->IR & (0x1 << 0))
	{
		int delta[MAX_KEYS], i;
		LPC_CT16B1 ->IR = 0x1 << 0; /* clear interrupt flag */
		if (m_cPollEnable)
		{
			// read updated capture values
			meas[2] = LPC_CT16B0 ->CR0;
			meas[1] = LPC_CT16B0 ->CR2;
			meas[0] = LPC_CT16B1 ->CR0;
			maxdelta = 0;
			for (i = 0; i < MAX_KEYS; i++)
			{
				if (fmeas[i] == 0)
					fmeas[i] = meas[i] << 15;
				else
					fmeas[i] = ((fmeas[i] >> 6) * 63) + (meas[i] << 9);
				count[i] = fmeas[i] >> 15;
				if (base[i] == 0)
					base[i] = count[i] << 15;
				delta[i] = count[i] - (base[i] >> 15);
				if (delta[i] > maxdelta)
					maxdelta = delta[i];
			}
			if (maxdelta > DEF_DELTA)
				threshold = maxdelta * 8 / 10;
			else
				threshold = DEF_DELTA;
			for (i = 0; i < MAX_KEYS; i++)
			{
				if (delta[i] > threshold)
					cnt[i]++;
				else if (cnt[i])
					cnt[i]--;
				if (cnt[i] >= KEY_DN_CNT)
				{
					if ((keystate & (1 << i)) == 0)
					{
						keystate |= 1 << i;
						event = i * 3 + keK1_PRESS;
						xQueueSendFromISR(eventq, &event, pdFALSE);
					}
					else if (cnt[i] == KEY_RPT_CNT)
					{
						cnt[i] = KEY_WAIT_CNT + acc[i] * KEY_INC_CNT;
						if (acc[i] < KEY_MAX_ACC)
							acc[i]++;
						event = i * 3 + keK1_REPEAT;
						xQueueSendFromISR(eventq, &event, pdFALSE);
					}
				}
				else if ((cnt[i] == 0) && (keystate & (1 << i)))
				{
					acc[i] = 0;
					keystate &= ~(1 << i);
					event = i * 3 + keK1_RELEASE;
					xQueueSendFromISR(eventq, &event, pdFALSE);
				}
				if (abs(delta[i]) < (threshold / 5) && !keystate)
					base[i] = ((base[i] >> 9) * 511) + (count[i] << 6);
			}
		}
	}
}

KEY_EVENT_t CAP_KEY_GetEvent(void)
{
	portBASE_TYPE ret;
	KEY_EVENT_t val;
	ret = xQueueReceive(eventq,&val,0);
	if (ret != pdPASS)
		val = keNONE;
	return val;
}

void EnableKeyPoll(char enable)
{
	m_cPollEnable = enable;
}
void CAP_KEY_Init(void)
{

	int TimerInterval = 65535;
	m_cPollEnable = 1;
	LPC_SYSCON ->SYSAHBCLKCTRL |= (1 << 7) | (1 << 8);

	LPC_CT16B0 ->PR = 0;
	LPC_CT16B0 ->MR0 = TimerInterval;
	LPC_CT16B0 ->MR1 = TimerInterval / 2;
	LPC_CT16B0 ->EMR = 0;
	LPC_CT16B0 ->MCR = 2; /* Reset on MR0 */
	LPC_CT16B0 ->PWMC = 0;
	LPC_CT16B0 ->CCR = 1 | (1 << 6);
	LPC_CT16B1 ->PR = 0;
	LPC_CT16B1 ->MR0 = TimerInterval;
	LPC_CT16B1 ->MR1 = TimerInterval / 2;
	LPC_CT16B1 ->EMR &= ~(0xFF << 4);
	LPC_CT16B1 ->EMR |= (0x3 << 4);
	LPC_CT16B1 ->MCR = 3; /* Interrupt and Reset on MR0 */
	LPC_CT16B1 ->PWMC = 2;
	LPC_CT16B1 ->CCR = 1 | (1 << 6);
	LPC_CT16B0 ->TCR = 3;
	LPC_CT16B1 ->TCR = 3;
	LPC_CT16B1 ->TCR = 1;
	LPC_CT16B0 ->TCR = 1;
	LPC_IOCON ->PIO1_16 = 0xa2;
	LPC_IOCON ->PIO1_17 = 0xa1;
	LPC_IOCON ->PIO0_20 = 0xa1;
	LPC_IOCON ->PIO1_23 = 0xa1;
	eventq = xQueueCreate(CAPKEY_QSIZE,sizeof(KEY_EVENT_t));
	NVIC_EnableIRQ(TIMER_16_1_IRQn);

}
