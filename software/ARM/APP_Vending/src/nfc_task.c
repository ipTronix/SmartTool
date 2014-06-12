#include "config.h"

#include <err.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include <nfc/nfc.h>

#include <freefare.h>
#include "FreeRTOS.h"
#include "semphr.h"
#include "CAP_KEY/CAP_KEY.h"
#include "GPIO/GPIO.h"

static nfc_context *context;
static nfc_device *pnd;

extern xQueueHandle xUIQueue;

uint8_t key_data_app[8] =
{ 0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF };

#define VENDING_SAMPLE_AID 0xF60000
int
mifare_desfire_getValue(MifareTag tag, int32_t *amount, uint32_t aid,
		uint8_t *key_data);
extern int nTransactionAmount;

void nfc_task(void * data)
{
	VCOM_start();
	nfc_init(&context);
	if (context == NULL )
	{
		//we should never get here!
		for (;;)
			;
	}
	pnd = nfc_open(context, NULL );

	for (;;)
	{
		MifareTag *tags = NULL;
		EnableKeyPoll(0);
		tags = freefare_get_tags(pnd);

		if (tags)
		{
			for (int i = 0; tags[i] && i < MAX_CANDIDATES; i++)
			{
				int32_t res;
				int32_t amount;
				switch (freefare_get_tag_type(tags[i]))
				{
					case DESFIRE:
						res = mifare_desfire_getValue(tags[i], &amount, VENDING_SAMPLE_AID,
								key_data_app);
						if (res < 0 && nTransactionAmount > 0)
						{
							// we are charging a card that doesn't have the vending AID.
							// let's create the application and initialize credit
							res = mifare_desfire_initialize_credit(tags[i], key_data_app,
									0xE000, VENDING_SAMPLE_AID, 0, 5000, nTransactionAmount, 0);
							nTransactionAmount = 0;
							xQueueSend(xUIQueue, &amount, portMAX_DELAY);
						}
						else
						{
							if (nTransactionAmount > 0)
							{
								// we're adding credit to card.
								amount += nTransactionAmount;
								res = mifare_desfire_exec_credit(tags[i], nTransactionAmount,
										VENDING_SAMPLE_AID, key_data_app);
								nTransactionAmount = 0;
								// send UI application updated credit
								xQueueSend(xUIQueue, &amount, portMAX_DELAY);
							}
							else if (nTransactionAmount < 0)
							{
								// we're subcracting credit
							  nTransactionAmount = -nTransactionAmount;
								if (amount - nTransactionAmount >= 0)
								{
									// credit on card is enough. let's subtract credit and
									// in case it's successful let's close MOSFET and send a message back.
									res = mifare_desfire_exec_debit(tags[i], nTransactionAmount,
											VENDING_SAMPLE_AID, key_data_app);
									if (res==0)
									{
										GPIOSetBitValue(PIN_MOSFET,1);
										amount -= nTransactionAmount;
										nTransactionAmount = 0;
										xQueueSend(xUIQueue, &amount, portMAX_DELAY);
									}

								}
								else
								{
									// invert value to indicate there's not enough credit
									amount = -amount;
									xQueueSend(xUIQueue, &amount, portMAX_DELAY);
								}
							}
						}
						break;
					default:
						continue;
				}
			}
			freefare_free_tags(tags);
		}
		nfc_idle(pnd);
		EnableKeyPoll(1);
		vTaskDelay(500);
	}
}
