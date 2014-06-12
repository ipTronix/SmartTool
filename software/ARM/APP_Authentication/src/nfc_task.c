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

#include "sample_secure_data.h"
#include <cr_section_macros.h>

static nfc_context *context;
static nfc_device *pnd;

extern xQueueHandle xUIQueue;

__BSS(RAM2) uint8_t NDEFWriteBuffer[NDEF_WRITE_MAX_LEN];

secure_payload_sample_t sample_payload;
uint8_t key_data_app[8] =
{ 0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF };

uint8_t g_bReceivedFromPhone;

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

const uint8_t ndef_default_msg[NDEF_MSG_LEN] = {
	0,    NDEF_MSG_LEN-2,
    0xd1, 0x02, 0x1d, 0x53, 0x70, 0x91, 0x01, 0x0a,
    0x55, 0x01, 0x61, 0x72, 0x72, 0x6f, 0x77, 0x2e,
    0x63, 0x6f, 0x6d, 0x55, 0x00, 0x0c, 0x61, 0x75,
    0x74, 0x68, 0x5f, 0x72, 0x65, 0x71, 0x75, 0x65,
    0x73, 0x74
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
  .ndef_file_written = NDEFWriteBuffer,
  .ndef_file_written_maxlen = NDEF_WRITE_MAX_LEN,
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

void nfc_task(void * data)
{
	nfc_init(&context);
	if (context == NULL)
	{
		//we should never get here!
		for (;;)
			;
	}

	for (;;)
	{
		MifareTag *tags = NULL;
		EnableKeyPoll(0);
		pnd = nfc_open(context, NULL);
		if (pnd != NULL)
		{
			tags = freefare_get_tags(pnd);

			if (tags)
			{
				for (int i = 0; tags[i] && i < MAX_CANDIDATES; i++)
				{
					uint32_t res;
					switch (freefare_get_tag_type(tags[i]))
					{
						case DESFIRE:
							res = mifare_desfire_read_secure(tags[i], &sample_payload,
									sizeof(sample_payload), SECURE_SAMPLE_AID, key_data_app);
							if ((res != sizeof(sample_payload)) || (m_eMode !=mdAUTHENTICATE))
							{
								if (m_eMode==mdAUTHORIZE)
								{
								// we couldn't read data.. react here
								res = mifare_desfire_initialize_secure(tags[i],
										sizeof(sample_payload), key_data_app, 0, SECURE_SAMPLE_AID);
								sample_payload.access_rights = 0xffffffff;
								strcpy(sample_payload.name, "test");
								res = mifare_desfire_write_secure(tags[i], &sample_payload,
										sizeof(sample_payload), SECURE_SAMPLE_AID, key_data_app);
								}
								else if (m_eMode==mdDENY)
								{
									// we couldn't read data.. react here
									res = mifare_desfire_initialize_secure(tags[i],
											sizeof(sample_payload), key_data_app, 0, SECURE_SAMPLE_AID);
									sample_payload.access_rights = 0x0000;
									strcpy(sample_payload.name, "test");
									res = mifare_desfire_write_secure(tags[i], &sample_payload,
											sizeof(sample_payload), SECURE_SAMPLE_AID, key_data_app);
								}
							}
							else
							{
								if (m_eMode==mdAUTHENTICATE)
								// data has been read. now inform UI
									xQueueSend(xUIQueue, &sample_payload, portMAX_DELAY);
							}
							break;
						default:
							continue;
					}
				}
				freefare_free_tags(tags);
			}
		}
		if (m_eMode==mdAUTHENTICATE)
		{
			g_bReceivedFromPhone = 0;
			pnd = nfc_open(context, NULL);
			if (pnd != NULL)
			{
				for (int i=0; i<NDEF_WRITE_MAX_LEN; i++)
				{
					NDEFWriteBuffer[i] = 0;
				}
				EnableKeyPoll(1);
				nfc_emulate_target(pnd, &emulator, 2000);
				EnableKeyPoll(0);
				if (g_bReceivedFromPhone)
				{
					strcpy(sample_payload.name, "phone access");
					if (memcmp(&NDEFWriteBuffer[24], "authenticate",12)==0)
					{
						sample_payload.access_rights = 0xffffffff;
					}
					else
					{
						sample_payload.access_rights = 0;
					}
					xQueueSend(xUIQueue, &sample_payload, portMAX_DELAY);
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
