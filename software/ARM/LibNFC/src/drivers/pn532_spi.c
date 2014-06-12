/*-
 * Free/Libre Near Field Communication (NFC) library
 *
 * Libnfc historical contributors:
 * Copyright (C) 2009      Roel Verdult
 * Copyright (C) 2009-2013 Romuald Conty
 * Copyright (C) 2010-2012 Romain Tartière
 * Copyright (C) 2010-2013 Philippe Teuwen
 * Copyright (C) 2012-2013 Ludovic Rousseau
 * Additional contributors of this file:
 * Copyright (C) 2013      Evgeny Boger
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
 */

/**
 * @file pn532_spi.c
 * @brief PN532 driver using SPI bus
 */

#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif // HAVE_CONFIG_H

#include "pn532_spi.h"

#include <stdio.h>
//#include <inttypes.h>
#include <string.h>
#include <nfc/nfc.h>

#include "drivers.h"
#include "nfc-internal.h"
#include "chips/pn53x.h"
#include "chips/pn53x-internal.h"
#include "SSP/ssp.h"
#include "GPIO/gpio.h"
#include "FreeRTOS.h"
#include "log.h"

#define PN532_SPI_DEFAULT_SPEED 1000000 // 1 MHz
#define PN532_SPI_DRIVER_NAME "pn532_spi"
#define PN532_SPI_MODE SPI_MODE_0

#define LOG_CATEGORY "libnfc.driver.pn532_spi"
#define LOG_GROUP    NFC_LOG_GROUP_DRIVER

// Internal data structs
const struct pn53x_io pn532_spi_io;
 struct pn532_spi_data {
// removed to save some memory
//  uint8_t port;
  volatile bool abort_flag;
};

static const uint8_t pn532_spi_cmd_dataread = PN532_SPI_CMD_DATAREAD;
static const uint8_t pn532_spi_cmd_datawrite = PN532_SPI_CMD_DATAWRITE;

uint8_t abtTmpBuf[6];

// Prototypes
int     pn532_spi_ack(nfc_device *pnd);
int     pn532_spi_wakeup(nfc_device *pnd);

#define DRIVER_DATA(pnd) ((struct pn532_spi_data*)(pnd->driver_data))

#ifdef NFC_STATIC_STRUCTS
static struct pn532_spi_data driver_data = {
  .abort_flag = false,
};

static nfc_modulation_type modulation_type[9];

static struct pn53x_data chip_data = {
  .type = PN532,
  .power_mode = LOWVBAT,
  .supported_modulation_as_initiator = modulation_type,
};

#endif

static size_t
pn532_spi_scan(const nfc_context *context, nfc_connstring connstrings[], const size_t connstrings_len)
{
// this function has been rewritten as we only have one device connected
/*
  size_t device_found = 0;
  spi_port sp;
  char **acPorts = spi_list_ports();
  const char *acPort;
  int     iDevice = 0;

  while ((acPort = acPorts[iDevice++])) {
    sp = spi_open(acPort);
    log_put(LOG_GROUP, LOG_CATEGORY, NFC_LOG_PRIORITY_DEBUG, "Trying to find PN532 device on SPI port: %s at %d Hz.", acPort, PN532_SPI_DEFAULT_SPEED);

    if ((sp != INVALID_SPI_PORT) && (sp != CLAIMED_SPI_PORT)) {
      // Serial port claimed but we need to check if a PN532_SPI is opened.
      spi_set_speed(sp, PN532_SPI_DEFAULT_SPEED);
      spi_set_mode(sp, PN532_SPI_MODE);

      nfc_connstring connstring;
      snprintf(connstring, sizeof(nfc_connstring), "%s:%s:%"PRIu32, PN532_SPI_DRIVER_NAME, acPort, PN532_SPI_DEFAULT_SPEED);
      nfc_device *pnd = nfc_device_new(context, connstring);
      if (!pnd) {
        perror("malloc");
        spi_close(sp);
        return 0;
      }
      pnd->driver = &pn532_spi_driver;
      pnd->driver_data = malloc(sizeof(struct pn532_spi_data));
      if (!pnd->driver_data) {
        perror("malloc");
        spi_close(sp);
        nfc_device_free(pnd);
        return 0;
      }
      DRIVER_DATA(pnd)->port = sp;

      // Alloc and init chip's data
      if (pn53x_data_new(pnd, &pn532_spi_io) == NULL) {
        perror("malloc");
        spi_close(DRIVER_DATA(pnd)->port);
        nfc_device_free(pnd);
        return 0;
      }
      // SAMConfiguration command if needed to wakeup the chip and pn53x_SAMConfiguration check if the chip is a PN532
      CHIP_DATA(pnd)->type = PN532;
      // This device starts in LowVBat power mode
      CHIP_DATA(pnd)->power_mode = LOWVBAT;

      DRIVER_DATA(pnd)->abort_flag = false;

      // Check communication using "Diagnose" command, with "Communication test" (0x00)
      int res = pn53x_check_communication(pnd);
      spi_close(DRIVER_DATA(pnd)->port);
      pn53x_data_free(pnd);
      nfc_device_free(pnd);
      if (res < 0) {
        continue;
      }

      memcpy(connstrings[device_found], connstring, sizeof(nfc_connstring));
      device_found++;

      // Test if we reach the maximum "wanted" devices
      if (device_found >= connstrings_len)
        break;
    }
  }
  iDevice = 0;
  while ((acPort = acPorts[iDevice++])) {
    free((void *)acPort);
  }
  free(acPorts);
  return device_found;
*/
  size_t device_found = 0;

  nfc_device *pnd = nfc_device_new(context, NULL);
  pnd->driver = &pn532_spi_driver;
#ifndef NFC_STATIC_STRUCTS
  pnd->driver_data = pvPortMalloc(sizeof(struct pn532_spi_data));
  if (!pnd->driver_data) {
    perror("malloc");
    nfc_device_free(pnd);
    return 0;
  }

  // Alloc and init chip's data
  if (pn53x_data_new(pnd, &pn532_spi_io) == NULL) {
    perror("malloc");
    nfc_device_free(pnd);
    return 0;
  }
#endif
  // SAMConfiguration command if needed to wakeup the chip and pn53x_SAMConfiguration check if the chip is a PN532
  CHIP_DATA(pnd)->type = PN532;
  // This device starts in LowVBat power mode
  CHIP_DATA(pnd)->power_mode = LOWVBAT;

  DRIVER_DATA(pnd)->abort_flag = false;

  // Check communication using "Diagnose" command, with "Communication test" (0x00)
  int res = pn53x_check_communication(pnd);
  pn53x_data_free(pnd);
#ifndef NFC_STATIC_STRUCTS
  nfc_device_free(pnd);
#endif
  if (res < 0) {
      return 0;
  }

  return 1;

}

struct pn532_spi_descriptor {
  char *port;
  uint32_t speed;
};

static void
pn532_spi_close(nfc_device *pnd)
{
  pn53x_idle(pnd);

  pn53x_data_free(pnd);
#ifndef NFC_STATIC_STRUCTS
  nfc_device_free(pnd);
#endif
}

static nfc_device *
pn532_spi_open(const nfc_context *context, const nfc_connstring connstring)
{
  nfc_device *pnd = NULL;
  pnd = nfc_device_new(context, connstring);
  if (!pnd) {
    //perror("malloc");
    return NULL;
  }

#ifdef NFC_STATIC_STRUCTS
  pnd->driver_data=&driver_data;
#else
  pnd->driver_data = pvPortMalloc(sizeof(struct pn532_spi_data));
  if (!pnd->driver_data) {
    perror("malloc");
    nfc_device_free(pnd);
    return NULL;
  }
#endif

  // Alloc and init chip's data
  if (pn53x_data_new(pnd, &pn532_spi_io) == NULL) {
    //perror("malloc");
#ifndef NFC_STATIC_STRUCTS
    nfc_device_free(pnd);
#endif
    return NULL;
  }
  // SAMConfiguration command if needed to wakeup the chip and pn53x_SAMConfiguration check if the chip is a PN532
  CHIP_DATA(pnd)->type = PN532;
  // This device starts in LowVBat mode
  CHIP_DATA(pnd)->power_mode = LOWVBAT;

  // empirical tuning
  CHIP_DATA(pnd)->timer_correction = 48;
  pnd->driver = &pn532_spi_driver;

  DRIVER_DATA(pnd)->abort_flag = false;

  // Check communication using "Diagnose" command, with "Communication test" (0x00)
  if (pn53x_check_communication(pnd) < 0) {
    nfc_perror(pnd, "pn53x_check_communication");
    return NULL;
  }

  pn53x_init(pnd);
  return pnd;
}

static int
pn532_spi_read_spi_status(nfc_device *pnd)
{
  static const uint8_t pn532_spi_statread_cmd = 0x02;

  uint8_t spi_status = 0;
  SSP_Send_Receive(PN532_SPI_PORT, &pn532_spi_statread_cmd, 1, &spi_status, 1,true);

  return spi_status;
}

int
pn532_spi_wakeup(nfc_device *pnd)
{

  // setting CS low for at least 2 msec will power up PN532
  SSP_SetCS(PN532_SPI_PORT,1);
  vTaskDelay(2);

  int res = 0;
  // Try to get byte from the SPI line. If PN532 is powered down, the byte will be 0xff (MISO line is high)
  uint8_t spi_byte = 0;
  SSP_Receive(PN532_SPI_PORT, &spi_byte, 1,true);

  log_put(LOG_GROUP, LOG_CATEGORY, NFC_LOG_PRIORITY_DEBUG, "Got %x byte from SPI line before wakeup", spi_byte);

  CHIP_DATA(pnd)->power_mode = NORMAL; // PN532 will be awake soon
  vTaskDelay(1);

  if (spi_byte == 0xff) {
    log_put(LOG_GROUP, LOG_CATEGORY, NFC_LOG_PRIORITY_DEBUG, "%s", "Wakeup is needed");
//    spi_set_speed(DRIVER_DATA(pnd)->port, 5000); // set slow speed

    SSP_SetCS(PN532_SPI_PORT,1);
    vTaskDelay(2);
    res = pn532_SAMConfiguration(pnd, PSM_NORMAL, 1000); // wakeup by sending SAMConfiguration, which works just fine

//    spi_set_speed(DRIVER_DATA(pnd)->port, prev_port_speed);
  }


  return res;
}

#define PN532_BUFFER_LEN (PN53x_EXTENDED_FRAME__DATA_MAX_LEN + PN53x_EXTENDED_FRAME__OVERHEAD)


static int
pn532_spi_wait_for_data(nfc_device *pnd, int timeout)
{
  //TODO implement interrupt check
  static const uint8_t pn532_spi_ready = 0x01;
  static const int pn532_spi_poll_interval = 10; //ms
  portTickType startTick = xTaskGetTickCount();

  int timer = 0;

  int ret;
  /*
  while ((ret = pn532_spi_read_spi_status(pnd)) != pn532_spi_ready) {
    if (ret < 0) {
      return ret;
    }
*/
  while (GPIOGetPinValue(PIN_NFC_IRQ)) {
    if (DRIVER_DATA(pnd)->abort_flag) {
      DRIVER_DATA(pnd)->abort_flag = false;
      return NFC_EOPABORTED;
    }

    if (timeout > 0 && ((xTaskGetTickCount()-startTick)>timeout)) {
        return NFC_ETIMEOUT;

      //vTaskDelay(pn532_spi_poll_interval);
    }
  }

  return NFC_SUCCESS;
}

#if 0

static int
pn532_spi_receive_next_chunk(nfc_device *pnd, uint8_t *pbtData, const size_t szDataLen)
{
  // According to datasheet, the entire read operation should be done at once
  // However, it seems impossible to do since the length of the frame is stored in the frame
  // itself and it's impossible to manually set CS to low between two read operations

  // It's possible to read the response frame in a series of read operations, provided
  // each read operation is preceded by SPI_DATAREAD byte from the host.

  // Unfortunately, the PN532 sends first byte of the second and successive response chunks
  // at the same time as host sends SPI_DATAREAD byte

  // Many hardware SPI implementations are half-duplex, so it's became impossible to read this
  // first response byte

  // The following hack is used here: we first try to receive data from PN532 without SPI_DATAREAD
  // and then begin full-featured read operation

  // The PN532 does not shift the internal register on the receive operation, which allows us to read the whole response

  // The example transfer log is as follows:
  // CS                  ..._/---\___________________________/---\________/------\_____________/-----\_________/---\____________/---...
  // MOSI (host=>pn532)  ...       0x03 0x00 0x00 0x00 0x00        0x00            0x03  0x00          0x00          0x03  0x00
  // MISO (pn532<=host)  ...       0x01 0x00 0xff 0x02 0xfe        0xd5            0xd5  0x15          0x16          0x16  0x00
  // linux send/receive             s     r    r   r    r           r               s     r              r             s    r
  //                                    |<--      data    -->|    |<-data->|           |<-data->|    |<-data->|           |<-data->|
  //                               |<--    first chunk    -->|    |<--       second chunk    -->|    |<--    third chunk        -->|

  //  The response frame is 0x00 0xff 0x02 0xfe 0xd5 0x15 0x16 0x00


  SSP_Receive(PN532_SPI_PORT, pbtData, 1,true);

  SSP_Send_Receive(PN532_SPI_PORT, &pn532_spi_cmd_dataread, 1, pbtData + 1, szDataLen - 1,true);

  return NFC_SUCCESS;
}
#endif

const uint8_t pn53x_long_preamble[3] = { 0x00, 0x00, 0xff };

static int
pn532_spi_receive(nfc_device *pnd, uint8_t *pbtData, const size_t szDataLen, int timeout)
{

 // uint8_t  abtRxBuf[5];
  size_t len;

  pnd->last_error = pn532_spi_wait_for_data(pnd, timeout);

  if (NFC_EOPABORTED == pnd->last_error) {
    return pn532_spi_ack(pnd);
  }

  if (pnd->last_error != NFC_SUCCESS) {
    log_put(LOG_GROUP, LOG_CATEGORY, NFC_LOG_PRIORITY_ERROR, "%s", "Unable to wait for SPI data. (RX)");
    goto error;
  }

  SSP_Send(PN532_SPI_PORT, (uint8_t *) &pn532_spi_cmd_dataread, 1,true);
  SSP_SetCS(PN532_SPI_PORT,1);
  SSP_Receive_naked(PN532_SPI_PORT, abtTmpBuf , 4,true);

  if (0 == (memcmp(abtTmpBuf, pn53x_long_preamble, 3))) {
    // long preamble

    // omit first byte
    for (size_t i = 0; i < 3; ++i) {
    	abtTmpBuf[i] = abtTmpBuf[i + 1];
    }

    // need one more byte
    //pnd->last_error =
    SSP_Receive_naked(PN532_SPI_PORT, abtTmpBuf+3 , 1,true);//pn532_spi_receive_next_chunk(pnd, abtRxBuf + 3, 1);
    if (pnd->last_error != 0) {
      log_put(LOG_GROUP, LOG_CATEGORY, NFC_LOG_PRIORITY_ERROR, "%s", "Unable to receive one more byte for long preamble frame. (RX)");
      goto error;
    }
  }


  if (0 != (memcmp(abtTmpBuf, pn53x_long_preamble+1, 2))) {
    log_put(LOG_GROUP, LOG_CATEGORY, NFC_LOG_PRIORITY_ERROR, "%s", " preamble+start code mismatch");
    pnd->last_error = NFC_EIO;
    goto error;
  }

  if ((0x01 == abtTmpBuf[2]) && (0xff == abtTmpBuf[3])) {
    // Error frame
	  SSP_Receive_naked(PN532_SPI_PORT,abtTmpBuf, 3,true);

    log_put(LOG_GROUP, LOG_CATEGORY, NFC_LOG_PRIORITY_ERROR, "%s", "Application level error detected");
    pnd->last_error = NFC_EIO;
    goto error;
  } else if ((0xff == abtTmpBuf[2]) && (0xff == abtTmpBuf[3])) {
    // Extended frame
    //pnd->last_error = pn532_spi_receive_next_chunk(pnd, abtRxBuf, 3);
	  SSP_Receive_naked(PN532_SPI_PORT,abtTmpBuf, 3,true);

    if (pnd->last_error != 0) {
      log_put(LOG_GROUP, LOG_CATEGORY, NFC_LOG_PRIORITY_ERROR, "%s", "Unable to receive data. (RX)");
      goto error;
    }
    // (abtRxBuf[0] << 8) + abtRxBuf[1] (LEN) include TFI + (CC+1)
    len = (abtTmpBuf[0] << 8) + abtTmpBuf[1] - 2;
    if (((abtTmpBuf[0] + abtTmpBuf[1] + abtTmpBuf[2]) % 256) != 0) {
      log_put(LOG_GROUP, LOG_CATEGORY, NFC_LOG_PRIORITY_ERROR, "%s", "Length checksum mismatch");
      pnd->last_error = NFC_EIO;
      goto error;
    }
  } else {
    // Normal frame
    if (256 != (abtTmpBuf[2] + abtTmpBuf[3])) {
      // TODO: Retry
      log_put(LOG_GROUP, LOG_CATEGORY, NFC_LOG_PRIORITY_ERROR, "%s", "Length checksum mismatch");
      pnd->last_error = NFC_EIO;
      goto error;
    }

    // abtRxBuf[3] (LEN) include TFI + (CC+1)
    len = abtTmpBuf[2] - 2;
  }
/*
  if (len > szDataLen) {
    log_put(LOG_GROUP, LOG_CATEGORY, NFC_LOG_PRIORITY_ERROR, "Unable to receive data: buffer too small. (szDataLen: %zu, len: %zu)", szDataLen, len);
    pnd->last_error = NFC_EIO;
    goto error;
  }
*/
  // TFI + PD0 (CC+1)

 // pnd->last_error = pn532_spi_receive_next_chunk(pnd, abtRxBuf, 2);
  SSP_Receive_naked(PN532_SPI_PORT,abtTmpBuf, 2,true);

  if (pnd->last_error != 0) {
    log_put(LOG_GROUP, LOG_CATEGORY, NFC_LOG_PRIORITY_ERROR, "%s", "Unable to receive data. (RX)");
    goto error;
  }

  if (abtTmpBuf[0] != 0xD5) {
    log_put(LOG_GROUP, LOG_CATEGORY, NFC_LOG_PRIORITY_ERROR, "%s", "TFI Mismatch");
    pnd->last_error = NFC_EIO;
    goto error;
  }

  if (abtTmpBuf[1] != CHIP_DATA(pnd)->last_command + 1) {
    log_put(LOG_GROUP, LOG_CATEGORY, NFC_LOG_PRIORITY_ERROR, "%s", "Command Code verification failed");
    pnd->last_error = NFC_EIO;
    goto error;
  }

  if (len) {
    //pnd->last_error = pn532_spi_receive_next_chunk(pnd, pbtData, len);
	  SSP_Receive_naked(PN532_SPI_PORT,pbtData, len,true);

    if (pnd->last_error != 0) {
      log_put(LOG_GROUP, LOG_CATEGORY, NFC_LOG_PRIORITY_ERROR, "%s", "Unable to receive data. (RX)");
      goto error;
    }
  }

  //pnd->last_error = pn532_spi_receive_next_chunk(pnd, abtRxBuf, 2);
  SSP_Receive_naked(PN532_SPI_PORT,abtTmpBuf, 2,true);

  if (pnd->last_error != 0) {
    log_put(LOG_GROUP, LOG_CATEGORY, NFC_LOG_PRIORITY_ERROR, "%s", "Unable to receive data. (RX)");
    goto error;
  }
  if (pbtData)
  {
	  uint8_t btDCS = (256 - 0xD5);
	  btDCS -= CHIP_DATA(pnd)->last_command + 1;
	  for (size_t szPos = 0; szPos < len; szPos++) {
		btDCS -= pbtData[szPos];
	  }

	  if (btDCS != abtTmpBuf[0]) {
		log_put(LOG_GROUP, LOG_CATEGORY, NFC_LOG_PRIORITY_ERROR, "%s", "Data checksum mismatch");
		pnd->last_error = NFC_EIO;
		goto error;
	  }
  }
  if (0x00 != abtTmpBuf[1]) {
    log_put(LOG_GROUP, LOG_CATEGORY, NFC_LOG_PRIORITY_ERROR, "%s", "Frame postamble mismatch");
    pnd->last_error = NFC_EIO;
    goto error;
  }
  // The PN53x command is done and we successfully received the reply
  SSP_SetCS(PN532_SPI_PORT,0);
  return len;
error:
  SSP_SetCS(PN532_SPI_PORT,0);
  return pnd->last_error;

}

static int
pn532_spi_send(nfc_device *pnd, const uint8_t *pbtData, const size_t szData, int timeout)
{

  int res = 0;

  switch (CHIP_DATA(pnd)->power_mode) {
    case LOWVBAT: {
      /** PN532C106 wakeup. */
      if ((res = pn532_spi_wakeup(pnd)) < 0) {
        return res;
      }
      // According to PN532 application note, C106 appendix: to go out Low Vbat mode and enter in normal mode we need to send a SAMConfiguration command
      if ((res = pn532_SAMConfiguration(pnd, PSM_NORMAL, 1000)) < 0) {
        return res;
      }
    }
    break;
    case POWERDOWN: {
      if ((res = pn532_spi_wakeup(pnd)) < 0) {
        return res;
      }
    }
    break;
    case NORMAL:
      // Nothing to do :)
      break;
  };


/*
uint8_t  abtFrame[PN532_BUFFER_LEN + 1] = { PN532_SPI_CMD_DATAWRITE, 0x00, 0x00, 0xff };       // SPI data transfer starts with DATAWRITE (0x01) byte,  Every packet must start with "00 00 ff"
size_t szFrame = 0;

  if ((res = pn53x_build_frame(abtFrame + 1, &szFrame, pbtData, szData)) < 0) {
    pnd->last_error = res;
    return pnd->last_error;
  }
  SSP_Send(PN532_SPI_PORT, abtFrame, szFrame,true);
*/
  //start communication

  SSP_SetCS(PN532_SPI_PORT,1);
  SSP_Send_naked(PN532_SPI_PORT,&pn532_spi_cmd_datawrite,1,true);
  SSP_Send_naked(PN532_SPI_PORT,pn53x_long_preamble,3,true);
  if (szData <= PN53x_NORMAL_FRAME__DATA_MAX_LEN) {
	  abtTmpBuf[0] = szData+1;
	  abtTmpBuf[1] = 256-(szData+1);
	  abtTmpBuf[2] = 0xD4;
	  SSP_Send_naked(PN532_SPI_PORT,abtTmpBuf,3,true);
  } else if (szData <= PN53x_EXTENDED_FRAME__DATA_MAX_LEN) {
	  abtTmpBuf[0] = 0xff;
	  abtTmpBuf[1] = 0xff;
	  abtTmpBuf[2] = (szData + 1) >> 8;
	  abtTmpBuf[3] = (szData + 1) & 0xff;
	  abtTmpBuf[4] = 256 - ((abtTmpBuf[2] + abtTmpBuf[3]) & 0xff);
	  abtTmpBuf[5] = 0xD4;
	  SSP_Send_naked(PN532_SPI_PORT,abtTmpBuf,6,true);
  } else {
      log_put(LOG_GROUP, LOG_CATEGORY, NFC_LOG_PRIORITY_ERROR, "We can't send more than %d bytes in a raw (requested: %" PRIdPTR ")", PN53x_EXTENDED_FRAME__DATA_MAX_LEN, szData);
      return NFC_ECHIP;
  }
  SSP_Send_naked(PN532_SPI_PORT,pbtData,szData,true);

	abtTmpBuf[0] = (256 - 0xD4);
	for (size_t szPos = 0; szPos < szData; szPos++) {
		abtTmpBuf[0] -= pbtData[szPos];
	}
  abtTmpBuf[1] = 0;
  SSP_Send_naked(PN532_SPI_PORT,abtTmpBuf,2,true);
  SSP_SetCS(PN532_SPI_PORT,0);

  res = pn532_spi_wait_for_data(pnd, timeout);
  if (res != NFC_SUCCESS) {
    log_put(LOG_GROUP, LOG_CATEGORY, NFC_LOG_PRIORITY_ERROR, "%s", "Unable to wait for SPI data. (RX)");
    pnd->last_error = res;
    return pnd->last_error;
  }

  SSP_Send_Receive(PN532_SPI_PORT, &pn532_spi_cmd_dataread, 1, abtTmpBuf, 6,true);


  if (pn53x_check_ack_frame(pnd, abtTmpBuf, 6) == 0) {
    // The PN53x is running the sent command
  } else {
	    //LOG_HEX("SSP","RX",abtTmpBuf,6);
    return pnd->last_error;
  }

  return NFC_SUCCESS;
}


int
pn532_spi_ack(nfc_device *pnd)
{

  const size_t ack_frame_len = (sizeof(pn53x_ack_frame) / sizeof(pn53x_ack_frame[0]));
  uint8_t ack_tx_buf [1 + ack_frame_len];

  ack_tx_buf[0] = pn532_spi_cmd_datawrite;
  memcpy(ack_tx_buf + 1, pn53x_ack_frame, ack_frame_len);


  SSP_Send(PN532_SPI_PORT, ack_tx_buf, ack_frame_len + 1,true);
  return NFC_SUCCESS;

}

static int
pn532_spi_abort_command(nfc_device *pnd)
{
  if (pnd) {
    DRIVER_DATA(pnd)->abort_flag = true;
  }

  return NFC_SUCCESS;
}

const struct pn53x_io pn532_spi_io = {
  .send       = pn532_spi_send,
  .receive    = pn532_spi_receive,
};

const struct nfc_driver pn532_spi_driver = {
  .name                             = PN532_SPI_DRIVER_NAME,
  .scan_type                        = INTRUSIVE,
  .scan                             = pn532_spi_scan,
  .open                             = pn532_spi_open,
  .close                            = pn532_spi_close,
  .strerror                         = pn53x_strerror,

  .initiator_init                   = pn53x_initiator_init,
  .initiator_init_secure_element    = pn532_initiator_init_secure_element,
  .initiator_select_passive_target  = pn53x_initiator_select_passive_target,
  .initiator_poll_target            = pn53x_initiator_poll_target,
  .initiator_select_dep_target      = pn53x_initiator_select_dep_target,
  .initiator_deselect_target        = pn53x_initiator_deselect_target,
  .initiator_transceive_bytes       = pn53x_initiator_transceive_bytes,
  .initiator_transceive_bits        = pn53x_initiator_transceive_bits,
  .initiator_transceive_bytes_timed = pn53x_initiator_transceive_bytes_timed,
  .initiator_transceive_bits_timed  = pn53x_initiator_transceive_bits_timed,
  .initiator_target_is_present      = pn53x_initiator_target_is_present,

  .target_init           = pn53x_target_init,
  .target_send_bytes     = pn53x_target_send_bytes,
  .target_receive_bytes  = pn53x_target_receive_bytes,
  .target_send_bits      = pn53x_target_send_bits,
  .target_receive_bits   = pn53x_target_receive_bits,

  .device_set_property_bool     = pn53x_set_property_bool,
  .device_set_property_int      = pn53x_set_property_int,
  .get_supported_modulation     = pn53x_get_supported_modulation,
  .get_supported_baud_rate      = pn53x_get_supported_baud_rate,
#if 0
  .device_get_information_about = pn53x_get_information_about,
#endif

  .abort_command  = pn532_spi_abort_command,
  .idle           = pn53x_idle,
  .powerdown      = pn53x_PowerDown,
};

