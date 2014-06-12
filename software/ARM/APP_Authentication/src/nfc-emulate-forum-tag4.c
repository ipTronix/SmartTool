/*
 * nfc-emulate-forum-tag4.c
 *
 *  Created on: 21 Aug 2013
 *      Author: dario
 */


#include <string.h>

#include <nfc/nfc.h>
#include <nfc/nfc-emulation.h>
#include "nfc-emulate-forum-tag4.h"
#include <cr_section_macros.h>

#define type4v 2

extern uint8_t g_bReceivedFromPhone;

int
nfcforum_tag4_io(struct nfc_emulator *emulator, const uint8_t *data_in, const size_t data_in_len, uint8_t *data_out, const size_t data_out_len)
{
  int res = 0;

  struct nfcforum_tag4_ndef_data *ndef_data = (struct nfcforum_tag4_ndef_data *)(emulator->user_data);
  struct nfcforum_tag4_state_machine_data *state_machine_data = (struct nfcforum_tag4_state_machine_data *)(emulator->state_machine->data);

  if (data_in_len == 0) {
    // No input data, nothing to do
    return res;
  }

  if (data_in_len >= 4) {
    if (data_in[CLA] != 0x00)
      return -1;

#define ISO7816_SELECT         0xA4
#define ISO7816_READ_BINARY    0xB0
#define ISO7816_UPDATE_BINARY  0xD6

    switch (data_in[INS]) {
      case ISO7816_SELECT:

        switch (data_in[P1]) {
          case 0x00: /* Select by ID */
            if ((data_in[P2] | 0x0C) != 0x0C)
              return -1;

            const uint8_t ndef_capability_container[] = { 0xE1, 0x03 };
            const uint8_t ndef_file[] = { 0xE1, 0x04 };
            if ((data_in[LC] == sizeof(ndef_capability_container)) && (0 == memcmp(ndef_capability_container, data_in + DATA, data_in[LC]))) {
              memcpy(data_out, "\x90\x00", res = 2);
              state_machine_data->current_file = CC_FILE;
            } else if ((data_in[LC] == sizeof(ndef_file)) && (0 == memcmp(ndef_file, data_in + DATA, data_in[LC]))) {
              memcpy(data_out, "\x90\x00", res = 2);
              state_machine_data->current_file = NDEF_FILE;
            } else {
              memcpy(data_out, "\x6a\x00", res = 2);
              state_machine_data->current_file = NONE;
            }

            break;
          case 0x04: /* Select by name */
            if (data_in[P2] != 0x00)
              return -1;

            const uint8_t ndef_tag_application_name_v1[] = { 0xD2, 0x76, 0x00, 0x00, 0x85, 0x01, 0x00 };
            const uint8_t ndef_tag_application_name_v2[] = { 0xD2, 0x76, 0x00, 0x00, 0x85, 0x01, 0x01 };
            if ((type4v == 1) && (data_in[LC] == sizeof(ndef_tag_application_name_v1)) && (0 == memcmp(ndef_tag_application_name_v1, data_in + DATA, data_in[LC])))
              memcpy(data_out, "\x90\x00", res = 2);
            else if ((type4v == 2) && (data_in[LC] == sizeof(ndef_tag_application_name_v2)) && (0 == memcmp(ndef_tag_application_name_v2, data_in + DATA, data_in[LC])))
              memcpy(data_out, "\x90\x00", res = 2);
            else
              memcpy(data_out, "\x6a\x82", res = 2);

            break;
          default:
            return -1;
        }

        break;
      case ISO7816_READ_BINARY:
        if ((size_t)(data_in[LC] + 2) > data_out_len) {
          return -2;
        }
        switch (state_machine_data->current_file) {
          case NONE:
            memcpy(data_out, "\x6a\x82", res = 2);
            break;
          case CC_FILE:
            memcpy(data_out, ndef_data->capability_container + (data_in[P1] << 8) + data_in[P2], data_in[LC]);
            memcpy(data_out + data_in[LC], "\x90\x00", 2);
            res = data_in[LC] + 2;
            break;
          case NDEF_FILE:
            memcpy(data_out, ndef_data->ndef_file + (data_in[P1] << 8) + data_in[P2], data_in[LC]);
            memcpy(data_out + data_in[LC], "\x90\x00", 2);
            res = data_in[LC] + 2;
            break;
        }
        break;

      case ISO7816_UPDATE_BINARY:
      	if ((((uint16_t)data_in[P1] << 8) + data_in[P2] + data_in[LC]) <= ndef_data->ndef_file_written_maxlen)
      	{
					memcpy(ndef_data->ndef_file_written + (data_in[P1] << 8) + data_in[P2], data_in + DATA, data_in[LC]);
					//if ((data_in[P1] << 8) + data_in[P2] == 0) {
						//ndef_data->ndef_file_len = (ndef_data->ndef_file[0] << 8) + ndef_data->ndef_file[1] + 2;
					//}
      	}
        memcpy(data_out, "\x90\x00", res = 2);
        if (data_in[LC] == 2){
        	g_bReceivedFromPhone = 1;
        }
        break;
      default: // Unknown
        res = -1;
    }
  } else {
    res = -1;
  }

  return res;
}
