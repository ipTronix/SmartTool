/*
 * taginfo.h
 *
 *  Created on: 9 Aug 2013
 *      Author: dario
 */

#ifndef TAGINFO_H_
#define TAGINFO_H_

#define MAX_NDEF_MSG_SIZE 64

typedef struct
{
	  uint8_t  szUidLen;
	  uint8_t  abtUid[10];
	  uint8_t  msg[MAX_NDEF_MSG_SIZE];
	  uint8_t  msgLen;
} taginfo_t;

#endif /* TAGINFO_H_ */
