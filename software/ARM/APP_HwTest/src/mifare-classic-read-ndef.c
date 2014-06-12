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

#define MIN(a,b) ((a < b) ? a: b)

int
mifare_classic_read_ndef(MifareTag tag, char * buffer, char *max_size)
{
    int error = EXIT_SUCCESS;
    Mad mad;


	// NFCForum card has a MAD, load it.
	if (0 == mifare_classic_connect (tag)) {
	} else {
		return -2;
	}

	if ((mad = mad_read (tag))) {
		// Dump the NFCForum application using MAD information
		ssize_t len;
		if ((len = mifare_application_read (tag, mad, mad_nfcforum_aid, buffer, max_size, mifare_classic_nfcforum_public_key_a, MFC_KEY_A)) != -1) {
			uint8_t tlv_type;
			uint16_t tlv_data_len;
			uint8_t * tlv_data;
			uint8_t * pbuffer = buffer;
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
	} else {
		printf ("No MAD detected.\n");
	}
	vPortFree (mad);

	return error;
}
