/*
 * Copyright (c) 2025 Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the License); you may
 * not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an AS IS BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

// SDS I/O Client

#include <string.h>

#include "cmsis_os2.h"

#include "sdsio.h"
#include "sdsio_client.h"


// Lock function
#ifndef SDSIO_NO_LOCK
static osMutexId_t lock_id;
static inline void sdsioLockCreate (void) {
  lock_id = osMutexNew(NULL);
}
static inline void sdsioLockDelete (void) {
  osMutexDelete(lock_id);
}
static inline void sdsioLock (void) {
  osMutexAcquire(lock_id, osWaitForever);
}
static inline void sdsioUnLock (void) {
  osMutexRelease(lock_id);
}
#else
static inline void sdsioLockCreate (void) {}
static inline void sdsioLockDelete (void) {}
static inline void sdsioLock       (void) {}
static inline void sdsioUnLock     (void) {}
#endif

// SDS I/O functions

/** Initialize I/O interface */
int32_t sdsioInit (void) {
  int32_t ret;

  sdsioLockCreate();

  ret = sdsioClientInit();
  if (ret != SDSIO_OK) {
    sdsioLockDelete();
  }

  return ret;
}

/** Un-initialize I/O interface */
int32_t sdsioUninit (void) {
  sdsioClientUninit();
  sdsioLockDelete();
  return SDSIO_OK;
}

/**
  Open I/O stream
  Send:
    header: command   = SDSIO_CMD_OPEN
            sdsio_id  = not used
            argument  = sdsioMode_t
            data_size = size of stream name
    data:   stream name
  Receive:
    header: command   = SDSIO_CMD_OPEN
            sdsio_id  = retrieved sdsio identifier
            argument  = sdsioMode_t
            data_size = 0
    data:   no data
*/
sdsioId_t sdsioOpen (const char *name, sdsioMode_t mode) {
  uint32_t sdsio_id = 0U;
  header_t header;
  uint32_t size, data_size;

  if (name != NULL) {
    sdsioLock();

    data_size = strlen(name) + 1U;
    header.command   = SDSIO_CMD_OPEN;
    header.sdsio_id  = 0U;
    header.argument  = mode;
    header.data_size = data_size;

    // Send header + data
    size = sizeof(header_t) + data_size;
    if (sdsioClientSend(&header, name, data_size) == size) {

      // Receive header
      size = sizeof(header_t);
      if (sdsioClientReceive(&header, NULL, 0U) == size) {
        if ((header.command   == SDSIO_CMD_OPEN) &&
            (header.argument  == mode)           &&
            (header.data_size == 0U)) {
          sdsio_id = header.sdsio_id;
        }
      }
    }

    sdsioUnLock();
  }

  return (sdsioId_t)sdsio_id;
}

/**
  Close I/O stream.
  Send:
    header: command   = SDSIO_CMD_CLOSE
            sdsio_id  = sdsio identifier
            argument  = not used
            data_size = 0
    data:   no data
*/
int32_t sdsioClose (sdsioId_t id) {
  int32_t  ret = SDSIO_ERROR;
  header_t header;
  uint32_t size;

  if (id != NULL) {
    sdsioLock();

    header.command   = SDSIO_CMD_CLOSE;
    header.sdsio_id  = (uint32_t)id;
    header.argument  = 0U;
    header.data_size = 0U;

    // Send Header
    size = sizeof(header_t);
    if (sdsioClientSend(&header, NULL, 0U) == size) {
      ret = SDSIO_OK;
    }

    sdsioUnLock();
  }

  return ret;
}

/**
  Write data to I/O stream.
  Send:
    header: command   = SDSIO_CMD_WRITE
            sdsio_id  = sdsio identifier
            argument  = not used
            data_size = number of data bytes
    data:   data to be written
*/
uint32_t sdsioWrite (sdsioId_t id, const void *buf, uint32_t buf_size) {
  uint32_t num = 0U;
  header_t header;
  uint32_t size;

  if ((id != NULL) && (buf != NULL) && (buf_size != 0U)) {
    sdsioLock();

    header.command   = SDSIO_CMD_WRITE;
    header.sdsio_id  = (uint32_t)id;
    header.argument  = 0U;
    header.data_size = buf_size;

    // Send header + data
    size = sizeof(header_t) + buf_size;
    if (sdsioClientSend(&header, buf, buf_size) == size) {
      num = buf_size;
    }

    sdsioUnLock();
  }

  return num;
}

/**
  Read data from I/O stream.
  Send:
    header: command   = SDSIO_CMD_READ
            sdsio_id  = sdsio identifier
            argument  = number of bytes to be read
            data_size = 0
    data:   no data
  Receive:
    header: command   = SDSIO_CMD_READ
            sdsio_id  = sdsio identifier
            argument  = nonzero = end of stream, else 0
            data_size = number of data bytes read
    data    data read
*/
uint32_t sdsioRead (sdsioId_t id, void *buf, uint32_t buf_size) {
  uint32_t num = 0U;
  header_t header;

  if ((id != NULL) && (buf != NULL) && (buf_size != 0U)) {
    sdsioLock();

    header.command   = SDSIO_CMD_READ;
    header.sdsio_id  = (uint32_t)id;
    header.argument  = buf_size;
    header.data_size = 0U;

    // Send header
    if (sdsioClientSend(&header, NULL, 0U) == sizeof(header_t)) {

      // Receive header + data
      if (sdsioClientReceive(&header, buf, buf_size) >= sizeof(header_t)) {
        if ((header.command   == SDSIO_CMD_READ) &&
            (header.sdsio_id  == (uint32_t)id)   &&
            (header.data_size <= buf_size)) {

          // Note: End of stream information in response argument is currently not used

          num = header.data_size;
        }
      }
    }

    sdsioUnLock();
  }

  return num;
}

/**
  Check if end of stream has been reached.
  Send:
    header: command   = SDSIO_CMD_EOS
            sdsio_id  = sdsio identifier
            argument  = not used
            data_size = 0
    data:   no data
  Receive:
    header: command   = SDSIO_CMD_EOS
            sdsio_id  = sdsio identifier
            argument  = nonzero = end of stream, else 0
            data_size = 0
    data:   no data
*/
int32_t sdsioEndOfStream (sdsioId_t id) {
  int32_t  eos = 0;
  header_t header;

  if (id != NULL) {
    sdsioLock();

    header.command   = SDSIO_CMD_EOS;
    header.sdsio_id  = (uint32_t)id;
    header.argument  = 0U;
    header.data_size = 0U;

    // Send Header
    if (sdsioClientSend(&header, NULL, 0U) == sizeof(header_t)) {
      // Receive header
      if (sdsioClientReceive(&header, NULL, 0U) == sizeof(header_t)) {
        if ((header.command   == SDSIO_CMD_EOS) &&
            (header.sdsio_id  == (uint32_t)id)  &&
            (header.data_size == 0U)) {
          eos = (int32_t)header.argument;
        }
      }
    }
    sdsioUnLock();
  }

  return eos;
}
