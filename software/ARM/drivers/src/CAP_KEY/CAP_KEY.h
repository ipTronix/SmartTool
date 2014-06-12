/*
 * CAP_KEY.h
 *
 *  Created on: 13 Sep 2013
 *      Author: dario
 */

#ifndef CAP_KEY_H_
#define CAP_KEY_H_

typedef enum
{
	keK1_PRESS,
	keK1_REPEAT,
	keK1_RELEASE,
	keK2_PRESS,
	keK2_REPEAT,
	keK2_RELEASE,
	keK3_PRESS,
	keK3_REPEAT,
	keK3_RELEASE,
	keNONE,
} KEY_EVENT_t;

KEY_EVENT_t CAP_KEY_GetEvent(void);
void CAP_KEY_Init(void);
void EnableKeyPoll(char enable);

#endif /* CAP_KEY_H_ */
