#include "config.h"

#include <err.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include <nfc/nfc.h>
#include <nfc/nfc-emulation.h>
#include "nfc-emulate-forum-tag4.h"

#include <freefare.h>
#include "FreeRTOS.h"
#include "semphr.h"
#include "taginfo.h"
#include "freefare_internal.h"
#include "app_ndef_define.h"
#include <cr_section_macros.h>

taginfo_t taginfo;
uint8_t g_bReceivedFromPhone;
uint8_t  g_bDeliveredToPhone;

const uint8_t nfcforum_capability_container[] = {
  0x00, 0x0F, /* CCLEN 15 bytes */
  0x20,       /* Mapping version 2.0, use option -1 to force v1.0 */
  0x00, 0xf0, /* MLe Maximum R-ADPU data size */
// Notes:
//  - I (Romuald) don't know why Nokia 6212 Classic refuses the NDEF message if MLe is more than 0xFD (any suggests are welcome);
//  - ARYGON devices doesn't support extended frame sending, consequently these devices can't sent more than 0xFE bytes as APDU, so 0xFB APDU data bytes.
//  - I (Romuald) don't know why ARYGON device doesn't ACK when MLe > 0x54 (ARYGON frame length = 0xC2 (192 bytes))
  0x00, 0xFF, /* MLc Maximum C-ADPU data size */
  0x04,       /* T field of the NDEF File-Control TLV */
  0x06,       /* L field of the NDEF File-Control TLV */
  /* V field of the NDEF File-Control TLV */
  0xE1, 0x04, /* File identifier */
  0xFF, 0xFE, /* Maximum NDEF Size */
  0x00,       /* NDEF file read access condition */
  0x00,       /* NDEF file write access condition */
};

const uint8_t ndef_default_msg[32] = {
	0,    30,
    0xd1, 0x02, 0x19, 0x53, 0x70, 0x91, 0x01, 0x0a,
    0x55, 0x01, 0x61, 0x72, 0x72, 0x6f, 0x77, 0x2e,
    0x63, 0x6f, 0x6d, 0x55, 0x00, 0x08, 0x69, 0x70,
    0x54, 0x72, 0x6f, 0x6e, 0x69, 0x78
};

nfc_target nfc_nt = {
  .nm = {
    .nmt = NMT_ISO14443A,
    .nbr = NBR_UNDEFINED, // Will be updated by nfc_target_init()
  },
  .nti = {
    .nai = {
      .abtAtqa = { 0x00, 0x04 },
      .abtUid = { 0x08, 0x00, 0xb0, 0x0b },
      .szUidLen = 4,
      .btSak = 0x20,
      .abtAts = { 0x75, 0x33, 0x92, 0x03 }, /* Not used by PN532 */
      .szAtsLen = 4,
    },
  },
};

struct nfcforum_tag4_state_machine_data state_machine_data = {
  .current_file = NONE,
};

const struct nfcforum_tag4_ndef_data nfcforum_tag4_data = {
  .capability_container = nfcforum_capability_container,
  .ndef_file = ndef_default_msg,
  .ndef_file_len = NDEF_MSG_LEN,
  .ndef_file_written = taginfo.msg,
  .ndef_file_written_maxlen = MAX_NDEF_MSG_SIZE,
};

const struct nfc_emulation_state_machine state_machine = {
  .io   = nfcforum_tag4_io,
  .data = &state_machine_data,
};

const struct nfc_emulator emulator = {
  .target = &nfc_nt,
  .state_machine = &state_machine,
  .user_data = &nfcforum_tag4_data,
};

static nfc_context *context;
static nfc_device *pnd;

extern xQueueHandle xUIQueue;

void nfc_task(void * data)
{
	int len;

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

		if (g_eMode != modeEMULATE_CARD)
		{
			pnd = nfc_open(context, NULL );
			if (pnd != NULL)
			{
				tags = freefare_get_tags(pnd);

				if (tags)
				{
					for (int i = 0; tags[i] && i < MAX_CANDIDATES; i++)
					{
						taginfo.szUidLen = tags[i]->info.szUidLen;
						memcpy(taginfo.abtUid, tags[i]->info.abtUid, taginfo.szUidLen);
						switch (freefare_get_tag_type(tags[i]))
						{
							case ULTRALIGHT:
							case ULTRALIGHT_C:
								if (g_eMode == modeREAD_CARD)
								{
									taginfo.msgLen = mifare_ultralight_read_ndef(tags[i],&taginfo.msg,MAX_NDEF_MSG_SIZE);
								}
								else if (mifare_ultralight_write_ndef(tags[i],taginfo.msg) == 0)
								{
									xQueueSend(xUIQueue, &taginfo, portMAX_DELAY);
								}
								break;
							case CLASSIC_1K:
							case CLASSIC_4K:
								if (g_eMode == modeREAD_CARD)
								{
									taginfo.msgLen = mifare_classic_read_ndef(tags[i], &taginfo.msg,
																														MAX_NDEF_MSG_SIZE);
								}
								else if (mifare_classic_write_ndef(pnd, tags[i]) == 0)
								{
									xQueueSend(xUIQueue, &taginfo, portMAX_DELAY);
								}
								break;
							case DESFIRE:
								if (g_eMode == modeREAD_CARD)
								{
									taginfo.msgLen = mifare_desfire_read_ndef(tags[i], &taginfo.msg,
																														MAX_NDEF_MSG_SIZE);
								}
								else if (mifare_desfire_write_ndef(tags[i], 0, 0) == 0)
								{
									xQueueSend(xUIQueue, &taginfo, portMAX_DELAY);
								}
								break;
							default:
								continue;
						}
						if (taginfo.msgLen > 0 && g_eMode == modeREAD_CARD)
						{
							xQueueSend(xUIQueue, &taginfo, portMAX_DELAY);
						}
					}
					freefare_free_tags(tags);
				}
			}
		}
		else
		{
			g_bReceivedFromPhone = 0;
			g_bDeliveredToPhone = 0;
			pnd = nfc_open(context, NULL);
			if (pnd != NULL)
			{
				EnableKeyPoll(1);
				nfc_emulate_target(pnd, &emulator, 2000);
				EnableKeyPoll(0);
				if (g_bReceivedFromPhone || g_bDeliveredToPhone)
				{
					taginfo.msgLen = (g_bReceivedFromPhone)? (((uint16_t)taginfo.msg[0])<<8) | taginfo.msg[1] : 0;
					xQueueSend(xUIQueue, &taginfo, portMAX_DELAY);
				}
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
