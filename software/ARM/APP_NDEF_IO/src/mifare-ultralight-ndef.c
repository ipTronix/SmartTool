/*
 * mifare-ultralight-read-ndef.c
 *
 *  Created on: 11 Sep 2013
 *      Author: dario
 */


/*-
 * Copyright (C) 2011, Romain Tartiere, Romuald Conty.
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>
 *
 * $Id $
 */

/*
 * This implementation was written based on information provided by the
 * following documents:
 *
 * Mifare Std as NFC Forum Enabled,
 * Extensions for Mifare standard 1k/4k as NFC Forum Enable Tag
 *   Application note
 *   Revision 1.1 — 21 August 2007
 *
 * NXP Type MF1K/4K Tag Operation, NXP Semiconductors [ANNFC1K4K]
 *   Application Note
 *   Revision 1.1 — 21 August 2007
 *   Document Identifier 130410
 */

#include "config.h"

#include <err.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include <nfc/nfc.h>

#include <freefare.h>
#include "FreeRTOS.h"
#include "app_ndef_define.h"
#include "taginfo.h"

#define MIN(a,b) ((a < b) ? a: b)

int
mifare_ultralight_read_ndef(MifareTag tag, char * buffer, int maxlen)
{
	int i;
	int error = EXIT_SUCCESS;
	int pagecount =  (freefare_get_tag_type(tag)== ULTRALIGHT_C) ? 0x2c : 0x10;
	int res=0;

	if (pagecount*4> maxlen)
		pagecount = maxlen/4;

	if (0 == mifare_ultralight_connect (tag)) {
	} else {
		return -2;
	}
	for (i=0;i<pagecount ;i++)
	{
		res |= mifare_ultralight_read(tag, i, &buffer[i*4]);
	}

	if (res==0)
	{
			uint8_t tlv_type;
			uint16_t tlv_data_len;
			uint8_t * tlv_data;
			uint8_t * pbuffer = &buffer[16];
			uint8_t found = false;
			while (!found)
			{
				tlv_data = tlv_decode (pbuffer, &tlv_type, &tlv_data_len);
				switch (tlv_type) {
					case 0x00:
						printf ("NFC Forum application contains a \"NULL TLV\", Skipping...\n");	// According to [ANNFC1K4K], we skip this Tag to read further TLV blocks.
						pbuffer += tlv_record_length(pbuffer, NULL, NULL);
						if (pbuffer >= buffer + sizeof(buffer)) {
							error= -3;
							found = true;
						}
					break;
					case 0x03:
						printf ("NFC Forum application contains a \"NDEF Message TLV\".\n");
						found = true;
						error = tlv_data_len;
						memcpy(buffer,tlv_data,tlv_data_len);
					break;
					case 0xFD:
						printf ("NFC Forum application contains a \"Proprietary TLV\", Skipping...\n");	// According to [ANNFC1K4K], we can skip this TLV to read further TLV blocks.
						pbuffer += tlv_record_length(pbuffer, NULL, NULL);
						if (pbuffer >= buffer + sizeof(buffer)) {
							error= -4;
							found = true;
						}
					break;
					case 0xFE:
						printf ("NFC Forum application contains a \"Terminator TLV\", no available data.\n");
						error= -5;
						found = true;
					break;
					default:
						printf ("NFC Forum application contains an invalid TLV.\n");
						error= -6;
						found = true;
				}
			}

		} else {
			printf ("No NFC Forum application.\n");
			error= -7;
		}

	if (res == 0)
	{
		res = buffer[17];
		for (i=0; i<pagecount*4-18; i++)
		{
			buffer[i] = buffer[18+i];
		}
	}
	mifare_ultralight_disconnect(tag);

	return res;//error;
}

int
mifare_ultralight_write_ndef(MifareTag tag, char * buffer)
{
	int i;
	int pagecount =  (freefare_get_tag_type(tag)== ULTRALIGHT_C) ? 0x2c : 0x10;
	int res=0;
	int ndef_msg_len = NDEF_MSG_LEN-1;
	int maxBytesWrite = MIN(MAX_NDEF_MSG_SIZE,((pagecount-4)*4));

	if (ndef_msg_len+2 > maxBytesWrite)
	{
		ndef_msg_len = maxBytesWrite-2;
	}

	for (i=0; i<MAX_NDEF_MSG_SIZE; i++)
	{
		buffer[i] = 0;
	}

	buffer[0] = 0x03;	//NDEF message TLV
	for (i=1; i<=ndef_msg_len; i++)
	{
		buffer[i] = ndef_default_msg[i];
	}

	buffer[ndef_msg_len+1] = 0xFE;	//TLV terminator

	if (0 == mifare_ultralight_connect (tag)) {
	} else {
		return -2;
	}

	pagecount = (maxBytesWrite +3)/4;
	for (i=0;i<pagecount;i++)
	{
		res |= mifare_ultralight_write(tag, 4+i, &buffer[i*4]);
	}

	mifare_ultralight_disconnect(tag);

	return res;//error;
}
