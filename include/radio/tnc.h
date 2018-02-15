/**
 * @file tnc.h
 * @brief Terminal Node Controller
 * 
 * Copyright © 2015 epsilonRT, All rights reserved.
 * This software is governed by the CeCILL license <http://www.cecill.info>
 */

#ifndef _SYSIO_TNC_H_
#define _SYSIO_TNC_H_

#include <sysio/defs.h>

#ifdef __cplusplus
  extern "C" {
#endif
/* ========================================================================== */
#include <stdio.h>

/**
 *  @addtogroup radio_ax25
 *  @{
 *  @defgroup radio_ax25_tnc Contrôleur de noeud terminal Ax25
 *
 *  Ce module fournit les fonctions permettant de transmettre de gérer
 *  des contrôleur de noeud terminal Ax25. \n
 *  Le format des trames TNC est : \n
 *  @code SOH STX <TXT: ASCII Hex bytes> ETX CRC3..0 EOT @endcode
 *  @{
 */

/* constants ================================================================ */
#define TNC_SOH 1
#define TNC_STX 2
#define TNC_ETX 3
#define TNC_EOT 4
#define TNC_TXT 5
#define TNC_CRC 6

/**
 * List of error codes returned by the functions.
 */
typedef enum {

  TNC_SUCCESS                  =  0,
  TNC_CRC_ERROR                = -1,
  TNC_NOT_ENOUGH_MEMORY        = -2,
  TNC_OBJECT_NOT_FOUND         = -3,
  TNC_ILLEGAL_MSG              = -4,
  TNC_FILE_NOT_FOUND           = -5,
  TNC_IO_ERROR                 = -6

} eTncError;

//##############################################################################
//#                                                                            #
//#                              xTnc Class                                    #
//#                                                                            #
//##############################################################################

/**
 * Tnc Class
 */
typedef struct xTnc {

  int fin;      ///< stream used to access the physical input
  int fout;     ///< stream used to access the physical output
  uint8_t *rxbuf;    ///< buffer for payload
  size_t  len;        ///< buffer length
  uint16_t crc_rx;  ///< computed rx CRC
  uint16_t crc_tx;  ///< received from tx CRC
  int state;
  uint8_t cnt;
  uint8_t msb;
} xTnc;

/**
 * Create and initialize a new xTnc object
 *
 * This object should be deleted with vTncDelete()
 * @return pointer on the object, NULL on error
 */
xTnc * xTncNew (size_t iRxBufferSize);

/**
 * Delete a xTnc object
 *
 * @param p object to operate on.
 */
void vTncDelete (xTnc *p);

/**
 * Clear all fields of the object
 *
 * @return TNC_SUCCESS, negative value on error
 */
int iTncClear (xTnc *p);

/**
 * Sets the file descriptor used to gain access to the physical medium in input
 *
 * @param fdin file descriptor
 * @return xTnc initialized object, NULL if error.
 */
int iTncSetFdin (xTnc *p, int fdin);

/**
 * Sets the file descriptor used to gain access to the physical medium in output
 *
 * @param fdout file descriptor
 * @return xTnc initialized object, NULL if error.
 */
int iTncSetFdout (xTnc *p, int fdout);

/**
 * Check if there are any TNC messages to be processed.
 * This function read available characters from the medium and search for
 * any TNC messages.
 * If a message is found, this function return true and the message can be read
 * with iTncRead().
 * This function may be blocking if there are no available chars and the FILE
 * used in @a tnc to access the medium is configured in blocking mode.
 *
 * @param tnc TNC object to operate on.
 * @return true if , negative value on error
 */
int iTncPoll(xTnc *tnc);

/**
 * Send an TNC message
 *
 * @param tnc TNC object to operate on.
 * @param frame message to send
 * @return TNC_SUCCESS, negative value on error
 */
ssize_t iTncWrite (xTnc *tnc, const void *buf, size_t count);

/**
 * Returns the last error code.
 */
int iTncError (xTnc *tnc);

/**
 *  @}
 * @}
 */

/* ========================================================================== */
#ifdef __cplusplus
  }
#endif

#endif /* _SYSIO_TNC_H_ */
