/*
 * cdc.h
 *
 *  Created on: 27 Jun 2013
 *      Author: Dario Pennisi
 */

#ifndef CDC_H_
#define CDC_H_

void VCOM_start();
int VCOM_read();
int VCOM_write(char *pcBuffer, int iLength);



#endif /* CDC_H_ */
