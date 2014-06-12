/*
 * sample_secure_data.h
 *
 *  Created on: Aug 10, 2013
 *      Author: dario
 */

#ifndef SAMPLE_SECURE_DATA_H_
#define SAMPLE_SECURE_DATA_H_

typedef struct
{
	char 		name[28];
	uint32_t	access_rights;
} secure_payload_sample_t;

typedef enum
{
	mdAUTHENTICATE,
	mdAUTHORIZE,
	mdDENY,
} MODE;

extern MODE m_eMode;

#define SECURE_SAMPLE_AID 0xF47000
#define SECURE_SAMPLE_ACCESS 0x0000

extern const uint8_t ndef_default_msg[];
#define NDEF_MSG_LEN	36

#define NDEF_WRITE_MAX_LEN 64
extern uint8_t NDEFWriteBuffer[NDEF_WRITE_MAX_LEN];

#endif /* SAMPLE_SECURE_DATA_H_ */
