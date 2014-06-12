/*
 * mifare-desfire-credit.c
 *
 *  Created on: Aug 10, 2013
 *      Author: dario
 */


#include "config.h"

#include <err.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include <nfc/nfc.h>

#include <freefare.h>
#include "FreeRTOS.h"


uint8_t key_data_picc[8] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

int
mifare_desfire_initialize_credit(MifareTag tag, uint8_t *key_data_app, uint16_t access_flags, uint32_t aid,int32_t lower_limit, int32_t upper_limit, int32_t value, uint8_t limited_credit_enable)
{
    int error = EXIT_SUCCESS;
    int res;
	uint8_t file_no;

    {

		res = mifare_desfire_connect (tag);
		if (res < 0) {
		    error = EXIT_FAILURE;
		    //break;
		}

		// We've to track DESFire version as NDEF mapping is different
		struct mifare_desfire_version_info info;
		res = mifare_desfire_get_version (tag, &info);
		if (res < 0) {
		    error = 1;
		    //break;
		}

		// Send Mifare DESFire Select Application with AID equal to 000000h to select the PICC level
		res = mifare_desfire_select_application(tag, NULL);
		if (res < 0)
		    errx (EXIT_FAILURE, "Application selection failed");

		MifareDESFireKey key_app;
		MifareDESFireKey key_picc;
		key_app  = mifare_desfire_des_key_new_with_version (key_data_app);
		key_picc  = mifare_desfire_des_key_new_with_version (key_data_picc);

		// Authentication with PICC master key MAY be needed to issue ChangeKeySettings command
		res = mifare_desfire_authenticate (tag, 0, key_picc);
		if (res < 0)
		    errx (EXIT_FAILURE, "Authentication with PICC master key failed");

		MifareDESFireAID aid_handle;
		uint8_t key_settings;
		uint8_t max_keys;
		mifare_desfire_get_key_settings(tag, &key_settings,&max_keys);
		if ((key_settings & 0x08) == 0x08){

		// Send Mifare DESFire ChangeKeySetting to change the PICC master key settings into :
		// bit7-bit4 equal to 0000b
		// bit3 equal to 1b, the configuration of the PICC master key MAY be changeable or frozen
		// bit2 equal to 1b, CreateApplication and DeleteApplication commands are allowed without PICC master key authentication
		// bit1 equal to 1b, GetApplicationIDs, and GetKeySettings are allowed without PICC master key authentication
		// bit0 equal to 1b, PICC masterkey MAY be frozen or changeable
		res = mifare_desfire_change_key_settings (tag,0x0F);
		if (res < 0)
			errx (EXIT_FAILURE, "ChangeKeySettings failed");
		}
		// Mifare DESFire Create Application with key settings equal to 0x09, NumOfKeys equal to 01h
		aid_handle = mifare_desfire_aid_new(aid);
		res = mifare_desfire_create_application (tag, aid_handle, 0x09, 1);
		if (res < 0)
		errx (EXIT_FAILURE, "Application creation failed. Try mifare-desfire-format before running");
		// Mifare DESFire SelectApplication (Select previously creates application)
		res = mifare_desfire_select_application(tag, aid_handle);
		if (res < 0)
		errx (EXIT_FAILURE, "Application selection failed");
		vPortFree (aid_handle);
		// Authentication with NDEF Tag Application master key (Authentication with key 0).
		// after creation key is blank so we can use picc key
		res = mifare_desfire_authenticate (tag, 0, key_picc);
		if (res < 0)
		errx (EXIT_FAILURE, "Authentication with NDEF Tag Application master key failed");
		// Mifare DESFire ChangeKeySetting with key settings equal to 00001001b
		res = mifare_desfire_change_key_settings (tag,0x09);
		if (res < 0)
		errx (EXIT_FAILURE, "ChangeKeySettings failed");

		res = mifare_desfire_change_key (tag, 0, key_app, key_picc);
		if (res < 0)
		errx (EXIT_FAILURE, "ChangeKey failed");
		res = mifare_desfire_authenticate (tag, 0, key_app);
		if (res < 0)
		    errx (EXIT_FAILURE, "Authentication with PICC master key failed");


		// Mifare DESFire CreateStdDataFile with FileNo equal to 00h , ComSet equal to 00h,
		// AccesRights equal to 0000h (master key required for all access)
		res = mifare_desfire_create_value_file (tag, 0x00 , MDCM_PLAIN, access_flags, lower_limit, upper_limit, value, limited_credit_enable);

		if (res < 0)
		errx (EXIT_FAILURE, "CreateStdDataFile failed");

		mifare_desfire_key_free (key_app);
		mifare_desfire_key_free (key_picc);

		mifare_desfire_disconnect (tag);
	}
    return res;

}

int
mifare_desfire_exec_credit(MifareTag tag, int32_t amount, uint32_t aid, uint8_t *key_data)
{
	uint8_t *cc_data;
	uint8_t *ndef_msg;
    int error = EXIT_SUCCESS;
    int res;
	uint8_t file_no;

    {
		res = mifare_desfire_connect (tag);
		if (res < 0) {
		    error = EXIT_FAILURE;
		    //break;
		}
		MifareDESFireKey key_app;
		key_app  = mifare_desfire_des_key_new_with_version (key_data);

		MifareDESFireAID aid_handle;
		// Mifare DESFire Create Application with AID equal to F47000h, key settings equal to 0x09, NumOfKeys equal to 01h
		aid_handle = mifare_desfire_aid_new(aid);
		// Mifare DESFire SelectApplication
		res = mifare_desfire_select_application(tag, aid_handle);
		if (res < 0)
		errx (EXIT_FAILURE, "Application selection failed");
		vPortFree(aid_handle);
		// Authentication with NDEF Tag Application master key (Authentication with key 0)
		res = mifare_desfire_authenticate (tag, 0, key_app);
		if (res < 0)
		errx (EXIT_FAILURE, "Authentication with NDEF Tag Application master key failed");
		res = mifare_desfire_credit (tag, 0, amount);
		if (res < 0)
		errx (EXIT_FAILURE, "can't credit");
		res = mifare_desfire_commit_transaction (tag);
		if (res < 0)
		errx (EXIT_FAILURE, "can't commit");

		mifare_desfire_key_free (key_app);

		mifare_desfire_disconnect (tag);
	}
    return res;
}

int
mifare_desfire_exec_debit(MifareTag tag, int32_t amount, uint32_t aid, uint8_t *key_data)
{
	uint8_t *cc_data;
	uint8_t *ndef_msg;
    int error = EXIT_SUCCESS;
    int res;
	uint8_t file_no;

    {
		res = mifare_desfire_connect (tag);
		if (res < 0) {
		    error = EXIT_FAILURE;
		    //break;
		}
		MifareDESFireKey key_app;
		key_app  = mifare_desfire_des_key_new_with_version (key_data);

		MifareDESFireAID aid_handle;
		// Mifare DESFire Create Application with AID equal to F47000h, key settings equal to 0x09, NumOfKeys equal to 01h
		aid_handle = mifare_desfire_aid_new(aid);
		// Mifare DESFire SelectApplication
		res = mifare_desfire_select_application(tag, aid_handle);
		if (res < 0)
		errx (EXIT_FAILURE, "Application selection failed");
		vPortFree(aid_handle);
		// Authentication with NDEF Tag Application master key (Authentication with key 0)
		res = mifare_desfire_authenticate (tag, 0, key_app);
		if (res < 0)
		errx (EXIT_FAILURE, "Authentication with NDEF Tag Application master key failed");
		res = mifare_desfire_debit(tag, 0, amount);
		if (res < 0)
		errx (EXIT_FAILURE, "can't debit");
		res = mifare_desfire_commit_transaction (tag);
		if (res < 0)
		errx (EXIT_FAILURE, "can't commit");

		mifare_desfire_key_free (key_app);

		mifare_desfire_disconnect (tag);
	}
    return res;
}

int
mifare_desfire_getValue(MifareTag tag, int32_t *amount, uint32_t aid, uint8_t *key_data)
{
	uint8_t *cc_data;
	uint8_t *ndef_msg;
    int error = EXIT_SUCCESS;
    int res;
	uint8_t file_no;

    {
		res = mifare_desfire_connect (tag);
		if (res < 0) {
		    error = EXIT_FAILURE;
		    //break;
		}
		MifareDESFireKey key_app;
		key_app  = mifare_desfire_des_key_new_with_version (key_data);

		MifareDESFireAID aid_handle;
		// Mifare DESFire Create Application with AID equal to F47000h, key settings equal to 0x09, NumOfKeys equal to 01h
		aid_handle = mifare_desfire_aid_new(aid);
		// Mifare DESFire SelectApplication
		res = mifare_desfire_select_application(tag, aid_handle);
		vPortFree(aid_handle);
		if (res < 0)
		errx (EXIT_FAILURE, "Application selection failed");
		// Authentication with NDEF Tag Application master key (Authentication with key 0)
		res = mifare_desfire_authenticate (tag, 0, key_app);
		if (res < 0)
		errx (EXIT_FAILURE, "Authentication with NDEF Tag Application master key failed");
		res = mifare_desfire_get_value (tag, 0, amount);
		if (res < 0)
		errx (EXIT_FAILURE, "can't read credit");

		mifare_desfire_key_free (key_app);

		mifare_desfire_disconnect (tag);
	}
    return res;
}
