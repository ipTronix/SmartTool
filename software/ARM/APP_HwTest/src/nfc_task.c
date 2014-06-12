#include "config.h"

#include <err.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include <nfc/nfc.h>

#include <freefare.h>
#include "FreeRTOS.h"
#include "semphr.h"
#include "taginfo.h"
#include "freefare_internal.h"
#include <cr_section_macros.h>

#include "CAP_KEY/CAP_KEY.h"

taginfo_t taginfo;
uint8_t g_bReceivedFromPhone;
uint8_t  g_bDeliveredToPhone;

static nfc_context *context;
static nfc_device *pnd;

extern xQueueHandle xUIQueue;

void nfc_task(void * data)
{
	int len;
	int i;

	nfc_init(&context);
	if (context == NULL )
	{
		//we should never get here!
		for (;;)
			;
	}

	for (;;)
	{
		MifareTag *tags = NULL;
		EnableKeyPoll(0);

		pnd = nfc_open(context, NULL );
		if (pnd != NULL)
		{
			tags = freefare_get_tags(pnd);

			if (tags)
			{
				for (i = 0; tags[i] && i < MAX_CANDIDATES; i++)
				{
					taginfo.szUidLen = tags[i]->info.szUidLen;
					memcpy(taginfo.abtUid, tags[i]->info.abtUid, taginfo.szUidLen);
					switch (freefare_get_tag_type(tags[i]))
					{
						case ULTRALIGHT:
						case ULTRALIGHT_C:
								taginfo.msgLen = mifare_ultralight_read_ndef(tags[i],&taginfo.msg,MAX_NDEF_MSG_SIZE);
							break;
						case CLASSIC_1K:
						case CLASSIC_4K:
								taginfo.msgLen = mifare_classic_read_ndef(tags[i], &taginfo.msg,
																			MAX_NDEF_MSG_SIZE);
							break;
						case DESFIRE:
								taginfo.msgLen = mifare_desfire_read_ndef(tags[i], &taginfo.msg,
																			MAX_NDEF_MSG_SIZE);
							break;
						default:
							continue;
					}
					if (taginfo.msgLen > 0)
					{
						xQueueSend(xUIQueue, &taginfo, portMAX_DELAY);
					}
				}
				freefare_free_tags(tags);
			}
		}

		if (pnd != NULL)
		{
			nfc_idle(pnd);
		}
		EnableKeyPoll(1);
		vTaskDelay(500);
	}
}
