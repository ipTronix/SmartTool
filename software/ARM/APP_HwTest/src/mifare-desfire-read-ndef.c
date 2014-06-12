/*-
 * Copyright (C) 2010, Audrey Diacre.
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
 * $Id$
 */

#include "config.h"

#include <err.h>
#include <stdlib.h>
#include <string.h>

#include <freefare.h>
#include "FreeRTOS.h"

/*
 * This example was written based on information provided by the
 * following documents:
 *
 * Mifare DESFire as Type 4 Tag
 * NFC Forum Type 4 Tag Extensions for Mifare DESFire
 * Rev. 1.1 - 21 August 2007
 * Rev. 2.2 - 4 January 2012
 *
 */

// Note that it is using specific Desfire commands, not ISO7816 NDEF Tag Type4 commands




int
mifare_desfire_read_ndef(MifareTag tag, char * buffer, short max_size)
{
	int res = -1;
	uint8_t *cc_data;
	uint8_t *ndef_msg;
	uint16_t  ndef_msg_len;
	uint8_t key_data_app[8]  = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };


	res = mifare_desfire_connect (tag);
	if (res < 0) {
		return -2;
	}

	// We've to track DESFire version as NDEF mapping is different
	struct mifare_desfire_version_info info;
	res = mifare_desfire_get_version (tag, &info);
	if (res < 0) {
		return -3;
	}

	MifareDESFireKey key_app;
	key_app  = mifare_desfire_des_key_new_with_version (key_data_app);

	// Mifare DESFire SelectApplication (Select application)
	MifareDESFireAID aid;
	if (info.software.version_major==0)
		aid = mifare_desfire_aid_new(0xEEEE10);
	else
		// There is no more relationship between DESFire AID and ISO AID...
		// Let's assume it's in AID 000001h as proposed in the spec
		aid = mifare_desfire_aid_new(0x000001);
	res = mifare_desfire_select_application(tag, aid);
	vPortFree(aid);
	if (res < 0)
	{
		mifare_desfire_key_free (key_app);
		return -4;
	}

	// Authentication with NDEF Tag Application master key (Authentication with key 0)
	res = mifare_desfire_authenticate (tag, 0, key_app);
	if (res < 0)
	{
		mifare_desfire_key_free (key_app);
		return -5;
	}

	// Read Capability Container file E103
	uint8_t lendata[20]; // cf FIXME in mifare_desfire.c read_data()
	if (info.software.version_major==0)
		res = mifare_desfire_read_data (tag, 0x03, 0, 2, lendata);
	else
		// There is no more relationship between DESFire FID and ISO FileID...
		// Let's assume it's in FID 01h as proposed in the spec
		res = mifare_desfire_read_data (tag, 0x01, 0, 2, lendata);
	if (res < 0)
	{
		mifare_desfire_key_free (key_app);
		return -6;
	}
	uint16_t cclen = (((uint16_t) lendata[0]) << 8) + ((uint16_t) lendata[1]);
	if (cclen < 15)
	{
		//CC too short IMHO
		mifare_desfire_key_free (key_app);
		return -7;
	}
	if (!(cc_data = pvPortMalloc(cclen+20))) // cf FIXME in mifare_desfire.c read_data()
	{
		mifare_desfire_key_free (key_app);
		return -8;
	}
	if (info.software.version_major==0)
		res = mifare_desfire_read_data (tag, 0x03, 0, cclen, cc_data);
	else
		res = mifare_desfire_read_data (tag, 0x01, 0, cclen, cc_data);
	if (res < 0)
	{
		vPortFree (cc_data);
		mifare_desfire_key_free (key_app);
		return -9;
	}
	// Search NDEF File Control TLV
	uint8_t off = 7;
	while (((off+7) < cclen) && (cc_data[off] != 0x04)) {
		// Skip TLV
		off += cc_data[off+1] + 2;
	}
	if (off+7 >= cclen)
	{
		//CC does not contain expected NDEF File Control TLV
		vPortFree (cc_data);
		mifare_desfire_key_free (key_app);
		return -10;
	}
	if (cc_data[off+2] != 0xE1)
	{
		//Unknown NDEF File reference in CC
		vPortFree (cc_data);
		mifare_desfire_key_free (key_app);
		return -11;
	}
	uint8_t file_no;
	if (info.software.version_major==0)
		file_no = cc_data[off+3];
	else
		// There is no more relationship between DESFire FID and ISO FileID...
		// Let's assume it's in FID 02h as proposed in the spec
		file_no = 2;
	uint16_t ndefmaxlen = (((uint16_t) cc_data[off+4]) << 8) + ((uint16_t) cc_data[off+5]);
	printf ("Max NDEF size: %i bytes\n", ndefmaxlen);

	res = mifare_desfire_read_data (tag, file_no, 0, 2, lendata);
	if (res < 0)
	{
		//Read NDEF len failed
		vPortFree (cc_data);
		mifare_desfire_key_free (key_app);
		return -13;
	}
	ndef_msg_len = (((uint16_t) lendata[0]) << 8) + ((uint16_t) lendata[1]);
	printf ("NDEF size: %i bytes\n", ndef_msg_len);
	if (ndef_msg_len + 2 > ndefmaxlen)
	{
		//Declared NDEF size larger than max NDEF size
		vPortFree (cc_data);
		mifare_desfire_key_free (key_app);
		return -14;
	}
	if ((max_size-1)<ndef_msg_len)
		ndef_msg_len = max_size-1;
	res = mifare_desfire_read_data (tag, file_no, 2, ndef_msg_len, buffer);
	if (res < 0)
	{
		//Read data failed
		vPortFree (cc_data);
		mifare_desfire_key_free (key_app);
		return -15;
	}
	res = ndef_msg_len;
	vPortFree (cc_data);
	mifare_desfire_key_free (key_app);

	mifare_desfire_disconnect (tag);
	return res;
}
