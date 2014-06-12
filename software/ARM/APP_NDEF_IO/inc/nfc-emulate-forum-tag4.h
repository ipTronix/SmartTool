/*
 * nfc-emulate-forum-tag4.h
 *
 *  Created on: 21 Aug 2013
 *      Author: dario
 */

#ifndef NFC_EMULATE_FORUM_TAG4_H_
#define NFC_EMULATE_FORUM_TAG4_H_

typedef enum { NONE, CC_FILE, NDEF_FILE } file;

struct nfcforum_tag4_ndef_data {
  uint8_t *ndef_file;
  size_t   ndef_file_len;
  uint8_t *capability_container;
  uint8_t *ndef_file_written;
  uint8_t ndef_file_written_maxlen;
};

struct nfcforum_tag4_state_machine_data {
  file     current_file;
};

/* C-ADPU offsets */
#define CLA  0
#define INS  1
#define P1   2
#define P2   3
#define LC   4
#define DATA 5

#define ISO144434A_RATS 0xE0

int
nfcforum_tag4_io(struct nfc_emulator *emulator, const uint8_t *data_in, const size_t data_in_len, uint8_t *data_out, const size_t data_out_len);

#endif /* NFC_EMULATE_FORUM_TAG4_H_ */
