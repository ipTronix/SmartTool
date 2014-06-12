/*
 * cdc.c
 *
 *  Created on: 27 Jun 2013
 *      Author: dario
 */

#include "LPC11Uxx.h"
#include "mw_usbd_rom_api.h"
#include "power_api.h"
#include <string.h>
#include "FreeRTOS.h"
#include "queue.h"
#include "config.h"

extern uint8_t VCOM_DeviceDescriptor[];
extern uint8_t VCOM_StringDescriptor[];
extern uint8_t VCOM_ConfigDescriptor[];

USBD_API_T* pUsbApi;

/* VCOM defines */
#define VCOM_BUFFERS    4
#define VCOM_BUF_EMPTY_INDEX  (0xFF)
#define VCOM_BUF_FREE   0
#define VCOM_BUF_ALLOC  1
#define VCOM_BUF_USBTXQ  2
#define VCOM_BUF_UARTTXQ  3
#define VCOM_BUF_ALLOCU  4

struct VCOM_DATA;
typedef void (*VCOM_SEND_T)(struct VCOM_DATA* pVcom);

typedef struct VCOM_DATA {
	USBD_HANDLE_T hUsb;
	USBD_HANDLE_T hCdc;
	uint8_t* rxBuf;
	uint8_t* txBuf;
	volatile uint16_t rxpos;
	volatile uint16_t rxlen;
	volatile uint16_t txlen;
	volatile uint8_t txpend;
	VCOM_SEND_T send_fn;
	volatile uint32_t sof_counter;
	volatile uint32_t last_ser_rx;
	volatile uint16_t break_time;
	volatile uint16_t usbrx_pend;
	volatile uint16_t config;
	xQueueHandle txq;
	xQueueHandle rxq;
} VCOM_DATA_T;

VCOM_DATA_T g_vCOM;
ErrorCode_t VCOM_bulk_out_hdlr(USBD_HANDLE_T hUsb, void* data, uint32_t event);
/**********************************************************************
 ** Function prototyping
 **********************************************************************/
void USB_pin_clk_init(void) {
	/* Enable AHB clock to the GPIO domain. */
	LPC_SYSCON ->SYSAHBCLKCTRL |= (1 << 6);

	/* Enable AHB clock to the USB block and USB RAM. */LPC_SYSCON ->SYSAHBCLKCTRL |=
			((0x1 << 14) | (0x1 << 27));

	/* Pull-down is needed, or internally, VBUS will be floating. This is to
	 address the wrong status in VBUSDebouncing bit in CmdStatus register. It
	 happens on the NXP Validation Board only that a wrong ESD protection chip is used. */LPC_IOCON ->PIO0_3 &=
			~0x1F;
//  LPC_IOCON->PIO0_3   |= ((0x1<<3)|(0x01<<0));	/* Secondary function VBUS */
	LPC_IOCON ->PIO0_3 |= (0x01 << 0); /* Secondary function VBUS */
	LPC_IOCON ->PIO0_6 &= ~0x07;
	LPC_IOCON ->PIO0_6 |= (0x01 << 0); /* Secondary function SoftConn */

	return;
}

void VCOM_usb_send(VCOM_DATA_T* pVcom) {
	/* data received send it back */
	if (pVcom->config) {
		pVcom->txlen -= pUsbApi->hw->WriteEP(pVcom->hUsb, USB_CDC_EP_BULK_IN,
				pVcom->txBuf, pVcom->txlen);
		pVcom->txpend = 1;
	}
}

ErrorCode_t VCOM_SendBreak(USBD_HANDLE_T hCDC, uint16_t mstime) {
	VCOM_DATA_T* pVcom = &g_vCOM;
	uint8_t lcr = LPC_USART ->LCR;

	if (mstime) {
		lcr |= (1 << 6);
	} else {
		lcr &= ~(1 << 6);
	}

	pVcom->break_time = mstime;
	LPC_USART ->LCR = lcr;

	return LPC_OK;
}

ErrorCode_t VCOM_bulk_in_hdlr(USBD_HANDLE_T hUsb, void* data, uint32_t event) {
	//VCOM_DATA_T* pVcom = (VCOM_DATA_T*) data;
//  Not needed as WriteEP() is called in VCOM_usb_send() immediately.
//  if (event == USB_EVT_IN) {
//	  VCOM_usb_send(&g_vCOM);
//  }
	int i;
	portBASE_TYPE num = uxQueueMessagesWaiting(g_vCOM.txq);
	if (num) {
		for (i = 0; i < num; i++) {
			xQueueReceive(g_vCOM.txq, &g_vCOM.txBuf[i], 0);
		}
		g_vCOM.txlen = num;
		VCOM_usb_send(&g_vCOM);
	} else {
		g_vCOM.txpend = 0;
	}
	return LPC_OK;
}

ErrorCode_t VCOM_bulk_out_hdlr(USBD_HANDLE_T hUsb, void* data, uint32_t event) {
	VCOM_DATA_T* pVcom = (VCOM_DATA_T*) data;
	int i;
	switch (event) {
	case USB_EVT_OUT:

		pVcom->rxlen = pUsbApi->hw->ReadEP(hUsb, USB_CDC_EP_BULK_OUT,
				pVcom->rxBuf);
		for (i = 0; i < pVcom->rxlen; i++) {
			xQueueSend(pVcom->rxq, &pVcom->rxBuf[i], 0);
		}
		break;
	default:
		break;
	}
	return LPC_OK;
}

void USB_IRQHandler(void) {
	pUsbApi->hw->ISR(g_vCOM.hUsb);
}

int VCOM_write(char *pcBuffer, int iLength) {
	if (g_vCOM.txq)
	{
		if (!g_vCOM.txpend && g_vCOM.config) {
			memcpy(g_vCOM.txBuf, pcBuffer, iLength);
			g_vCOM.txlen = iLength;
			VCOM_usb_send(&g_vCOM);
		} else {
			int i;
	//		  portTickType t = xTaskGetTickCount();
	//		  portBASE_TYPE num = uxQueueMessagesWaiting(g_vCOM.txq);
	//		  if (num<USB_HS_MAX_BULK_PACKET || t!=g_vCOM.last_tx_tick)
	//		  {
	//			  g_vCOM.last_tx_tick = t;
			if (g_vCOM.config) {
				for (i = 0; i < iLength; i++) {
					if (errQUEUE_FULL
							== xQueueSend(g_vCOM.txq,pcBuffer++,PRINTF_MAX_DELAY)) {
						xQueueReset(g_vCOM.txq);
						break;
					}

				}
			}
		}
	}
	return LPC_OK;

}

char VCOM_read() {
	char c;
	xQueueReceive(g_vCOM.rxq, &c, portMAX_DELAY);
	return c;
}

ErrorCode_t VCOM_Config_hdlr(USBD_HANDLE_T hUsb) {
	g_vCOM.config = 1;
	return LPC_OK;
}

void VCOM_start() {
	ErrorCode_t ret = LPC_OK;
	USBD_API_INIT_PARAM_T usb_param;
	USBD_CDC_INIT_PARAM_T cdc_param;
	USB_CORE_DESCS_T desc;
	USBD_HANDLE_T hUsb, hCdc;
	uint32_t ep_indx;

	/* get USB API table pointer */
	pUsbApi = (USBD_API_T*) ((*(ROM **) (0x1FFF1FF8))->pUSBD);

	/* enable clocks and pinmux for usb0 */
	USB_pin_clk_init();

	/* initilize call back structures */
	memset((void*) &usb_param, 0, sizeof(USBD_API_INIT_PARAM_T));
	usb_param.usb_reg_base = LPC_USB_BASE;
	usb_param.mem_base = 0x20004000;
	usb_param.mem_size = 0x800;
	usb_param.max_num_ep = 3;
	usb_param.USB_Configure_Event = VCOM_Config_hdlr;

	/* init CDC params */
	memset((void*) &cdc_param, 0, sizeof(USBD_CDC_INIT_PARAM_T));
	memset((void*) &g_vCOM, 0, sizeof(VCOM_DATA_T));

	/* user defined functions */
	cdc_param.SendBreak = VCOM_SendBreak;
	/* Initialize Descriptor pointers */
	memset((void*) &desc, 0, sizeof(USB_CORE_DESCS_T));
	desc.device_desc = (uint8_t *) &VCOM_DeviceDescriptor[0];
	desc.string_desc = (uint8_t *) &VCOM_StringDescriptor[0];
	desc.full_speed_desc = (uint8_t *) &VCOM_ConfigDescriptor[0];
	desc.high_speed_desc = (uint8_t *) &VCOM_ConfigDescriptor[0];

	/* USB Initialization */
	ret = pUsbApi->hw->Init(&hUsb, &desc, &usb_param);
	if (ret == LPC_OK) {
		// init CDC params
		cdc_param.mem_base = 0x20004500;
		cdc_param.mem_size = 0x300;
		cdc_param.cif_intf_desc =
				(uint8_t *) &VCOM_ConfigDescriptor[USB_CONFIGUARTION_DESC_SIZE];
		cdc_param.dif_intf_desc =
				(uint8_t *) &VCOM_ConfigDescriptor[USB_CONFIGUARTION_DESC_SIZE+
				USB_INTERFACE_DESC_SIZE + 0x0013 + USB_ENDPOINT_DESC_SIZE ];

		ret = pUsbApi->cdc->init(hUsb, &cdc_param, &hCdc);

		if (ret == LPC_OK) {
			/* store USB handle */
			g_vCOM.hUsb = hUsb;
			g_vCOM.hCdc = hCdc;
			g_vCOM.send_fn = VCOM_usb_send;

			/* allocate transfer buffers */
			g_vCOM.rxBuf = (uint8_t*) (cdc_param.mem_base
					+ (0 * USB_HS_MAX_BULK_PACKET));
			g_vCOM.txBuf = (uint8_t*) (cdc_param.mem_base
					+ (1 * USB_HS_MAX_BULK_PACKET));
			cdc_param.mem_size -= (4 * USB_HS_MAX_BULK_PACKET);

			g_vCOM.txq = xQueueCreate(USB_HS_MAX_BULK_PACKET,1);
			g_vCOM.rxq = xQueueCreate(USB_HS_MAX_BULK_PACKET*2,1);

			/* register endpoint interrupt  */
			ep_indx = (((USB_CDC_EP_BULK_IN & 0x0F) << 1) + 1);
			ret = pUsbApi->core->RegisterEpHandler(hUsb, ep_indx,
					VCOM_bulk_in_hdlr, &g_vCOM);
			if (ret == LPC_OK) {
				/* register endpoint interrupt  */
				ep_indx = ((USB_CDC_EP_BULK_OUT & 0x0F) << 1);
				ret = pUsbApi->core->RegisterEpHandler(hUsb, ep_indx,
						VCOM_bulk_out_hdlr, &g_vCOM);
				if (ret == LPC_OK) {
					/* enable IRQ */
					NVIC_EnableIRQ(USB_IRQn); //  enable USB0 interrrupts
					/* USB Connect */
					pUsbApi->hw->Connect(hUsb, 1);
				}
			}
		}
	}

}

