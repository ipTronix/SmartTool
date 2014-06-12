#ifndef APP_NDEF_DEFINE_H_
#define APP_NDEF_DEFINE_H_

typedef enum
{
	modeREAD_CARD,
	modeWRITE_CARD,
	modeEMULATE_CARD
}eMODE;

extern eMODE g_eMode;
extern uint8_t ndef_default_msg[];
#define NDEF_MSG_LEN	32

#endif /*APP_NDEF_DEFINE_H_*/
