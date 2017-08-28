/**
 * @file sysio/ax25.h
 * @brief Couche liaison du protocole AX.25
 * 
 * Copyright © 2015 epsilonRT, All rights reserved.
 * This software is governed by the CeCILL license <http://www.cecill.info>
 */

#ifndef _SYSIO_AX25_H_
#define _SYSIO_AX25_H_

#include <sysio/defs.h>

__BEGIN_C_DECLS
/* ========================================================================== */
#include <stdio.h>


/** 
 *  @addtogroup radio_group
 *  @{
 *  @defgroup radio_ax25 Liaisons Ax25
 *
 *  Ce module fournit les fonctions permettant de transmettre des informations
 *  sur des liaisons Ax25.
 *  @{
 */

/* constants ================================================================ */
/**
 * Maximum number of Repeaters in a AX25 message.
 */
#define AX25_MAX_RPT 8

/**
 * Maximum number of characters in a AX25 callsign.
 */
#define AX25_CALL_LEN 6

/**
 * Maximum number of bytes in a AX25 info field.
 */
#define AX25_INFO_LEN 256

/**
 * AX25 frame lenght without leading and trailling flags.
 */
#define AX25_FRAME_BUF_LEN ((AX25_CALL_LEN+1)*(2+AX25_MAX_RPT)+AX25_INFO_LEN+4)

/**
 * Control field: UI
 * This is the only value used in this lib so it is not possible to manage
 * frames of another type.
 */
#define AX25_CTRL_UI      0x03

/**
 * Protocol ID: No layer 3 protocol
 * This is the only value used in this lib so it is not possible to manage
 * frames of another type.
 */
#define AX25_PID_NOLAYER3 0xF0

/**
 * @name HDLC flags.
 * These should be moved in
 * a separated HDLC related file one day...
 */
#define HDLC_FLAG  0x7E
#define HDLC_RESET 0x7F
#define AX25_ESC   0x1B

/**
 * List of error codes returned by the functions.
 */
typedef enum {

  AX25_SUCCESS                  =  0,
  AX25_FILE_ACCESS_ERROR        = -1,
  AX25_ILLEGAL_CALLSIGN         = -2,
  AX25_INVALID_FRAME            = -3,
  AX25_CRC_ERROR                = -4,
  AX25_NOT_ENOUGH_MEMORY        = -5,
  AX25_NOT_ENOUGH_REPEATER      = -6,
  AX25_OBJECT_NOT_FOUND         = -7,
  AX25_FILE_NOT_FOUND           = -8,
  AX25_ILLEGAL_REPEATER         = -9,
  AX25_NO_FRAME_RECEIVED        = -10

} eAx25Error;

//##############################################################################
//#                                                                            #
//#                          xAx25Node Class                                   #
//#                                                                            #
//##############################################################################
/**
 * AX25 Node Class
 */
typedef struct xAx25Node {

  char callsign[AX25_CALL_LEN+1];  ///< Callsign, max 6 character
  uint8_t ssid; ///< SSID (secondary station ID) for the call
  bool flag; ///< boolean flag: has-been-repeated, general purpose ...
} xAx25Node;

/**
 * Create and initialize a new xAx25Node object
 *
 * This object should be deleted with vAx25NodeDelete()
 * @return pointer on the object, NULL on error
 */
xAx25Node * xAx25NodeNew (void);

/**
 * Delete a xAx25Node object
 *
 * @param n node object to operate on.
 */
void vAx25NodeDelete (xAx25Node *n);

/**
 * Initialize all fields of the node
 *
 * @param n node object to operate on.
 * @return AX25_SUCCESS, negative value on error
 */
int iAx25NodeClear (xAx25Node *n);

/**
 * Sets callsign of the node
 *
 * @param n node object to operate on.
 * @param callsign the new callsign (only the first 6 characters will be stored)
 * @return AX25_SUCCESS, negative value on error
 */
int iAx25NodeSetCallsign (xAx25Node *n, const char * callsign);

/**
 * Convert a node to a string for display
 *
 * The format is AAAAAA-S* where AAAAAA is the callsign, S the ssid if no null
 * and * is the has-been-repeated inficator.
 *
 * @param n node object to operate on.
 * @return The characters string should be deleted with free();
 */
char * xAx25NodeToStr (const xAx25Node *p);

/**
 * Print the node
 *
 * @param n node object to operate on.
 * @return AX25_SUCCESS, negative value on error
 */
int iAx25NodePrint (const xAx25Node *n);

/**
 * Print the node in the file
 *
 * @param n node object to operate on.
 * @return AX25_SUCCESS, negative value on error
 */
int iAx25NodeFilePrint (const xAx25Node *n, FILE * f);

//##############################################################################
//#                                                                            #
//#                           xAx25Frame Class                                 #
//#                                                                            #
//##############################################################################
/**
 * AX25 Frame Class
 * Used to handle AX25 sent/received messages.
 */
typedef struct xAx25Frame {

  xAx25Node *dst;                     ///< Destination node
  xAx25Node *src;                     ///< Source node
  xAx25Node *repeaters[AX25_MAX_RPT]; ///< List of the repeaters
  uint8_t repeaters_len;              ///< Number of repeaters
  uint8_t ctrl;                       ///< AX25 control field
  uint8_t pid;                        ///< AX25 PID field
  uint8_t info[AX25_FRAME_BUF_LEN+1]; ///< buffer for payload
  size_t  info_len;                    ///< payload length
} xAx25Frame;

/**
 * Create and initialize a new xAx25Frame object
 *
 * This object should be deleted with vAx25FrameDelete()
 * @return pointer on the object, NULL on error
 */
xAx25Frame * xAx25FrameNew (void);

/**
 * Delete a xAx25Node object
 *
 * @param n node object to operate on.
 */
void vAx25FrameDelete (xAx25Frame *frame);

/**
 * Clear the frame
 *
 * src and dst are cleared with iAx25NodeClear()
 * All the repeaters are removed.
 * The ctrl is set to AX25_CTRL_UI, the pid to AX25_PID_NOLAYER3.
 *
 * @return AX25_SUCCESS, negative value on error
 */
int iAx25FrameClear (xAx25Frame *frame);

/**
 * Sets the source address of frame
 *
 * @param frame AX25 frame object to operate on.
 * @param callsign Call string, max 6 character
 * @param SSID (secondary station ID) for the call
 * @return AX25_SUCCESS, negative value on error
 */
int iAx25FrameSetSrc (xAx25Frame *frame, const char *callsign, uint8_t ssid);

/**
 * Sets the destination address of frame
 *
 * @param frame AX25 frame object to operate on.
 * @param callsign Call string, max 6 character
 * @param SSID (secondary station ID) for the call
 * @return AX25_SUCCESS, negative value on error
 */
int iAx25FrameSetDst (xAx25Frame *frame, const char *callsign, uint8_t ssid);

/**
 * Adds a repeater to the frame
 *
 * @param frame AX25 frame object to operate on.
 * @param callsign Call string, max 6 character
 * @param SSID (secondary station ID) for the call
 * @return Index of the new repeater (0 to AX25_MAX_RPT-1), negative value on error
 */
int iAx25FrameAddRepeater (xAx25Frame *frame, const char *callsign, uint8_t ssid);

/**
 * Sets the repeater flag
 *
 * @param frame AX25 frame object to operate on.
 * @param index index of repeater
 * @param flag new flag value
 * @return AX25_SUCCESS, negative value on error
 */
int iAx25FrameSetRepeaterFlag (xAx25Frame *frame, uint8_t index, bool flag);

/**
 * Sets the info field of frame
 *
 * @param frame AX25 frame object to operate on.
 * @param callsign Call string, max 6 character
 * @param SSID (secondary station ID) for the call
 * @return AX25_SUCCESS, negative value on error
 */
int iAx25FrameSetInfo (xAx25Frame *frame, const void *info, size_t info_len);

/**
 * Convert a frame to a string for display
 *
 * @param frame AX25 frame object to operate on.
 * @return The characters string should be deleted with free();
 */
char * xAx25FrameToStr (const xAx25Frame *frame);

/**
 * Print a AX25 message in TNC-2 packet monitor format in a file
 * @param f a file channel where the message will be printed.
 * @param frame the message to be printed.
 */
int iAx25FrameFilePrint (const xAx25Frame *frame, FILE *f);

/**
 * Print a AX25 message in TNC-2 packet monitor format.
 * @param frame the message to be printed.
 */
int iAx25FramePrint (const xAx25Frame *frame);

//##############################################################################
//#                                                                            #
//#                             xAx25 Class                                    #
//#                                                                            #
//##############################################################################

/**
 * AX25 Controller Class
 */
typedef struct xAx25 {

  int fin;      ///< stream used to access the physical input
  int fout;     ///< stream used to access the physical output
  uint8_t buf[AX25_FRAME_BUF_LEN]; ///< buffer for payload
  size_t  len;    ///< buffer length
  uint16_t crc_in;  ///< CRC for current received frame
  uint16_t crc_out; ///< CRC of current sent frame
  bool sync;      ///< True if we have received a HDLC flag.
  bool escape;    ///< True when we have to escape the following char.
  bool frm_recv;  ///< True if we have received a valid frame
  int error;
} xAx25;

/**
 * Create and initialize a new xAx25 object
 *
 * This object should be deleted with vAx25Delete()
 * @return pointer on the object, NULL on error
 */
xAx25 * xAx25New (void);

/**
 * Delete a xAx25 object
 *
 * @param p object to operate on.
 */
void vAx25Delete (xAx25 *p);

/**
 * Clear all fields of the object
 *
 * @return AX25_SUCCESS, negative value on error
 */
int iAx25Clear (xAx25 *p);

/**
 * Sets the file descriptor used to gain access to the physical medium in input
 *
 * @param fdin file descriptor
 * @return xAx25 initialized object, NULL if error.
 */
int iAx25SetFdin (xAx25 *p, int fdin);

/**
 * Sets the file descriptor used to gain access to the physical medium in output
 *
 * @param fdout file descriptor
 * @return xAx25 initialized object, NULL if error.
 */
int iAx25SetFdout (xAx25 *p, int fdout);

/**
 * Check if there are any AX25 messages to be processed.
 * This function read available characters from the medium and search for
 * any AX25 messages.
 * If a message is found, this function return true and the message can be read
 * with iAx25Read().
 * This function may be blocking if there are no available chars and the FILE
 * used in @a ax25 to access the medium is configured in blocking mode.
 *
 * @param ax25 AX25 object to operate on.
 * @return true if , negative value on error
 */
bool bAx25Poll(xAx25 *ax25);

/**
 * Send an AX25 frame on the channel through a specific frame.
 *
 * @param ax25 AX25 object to operate on.
 * @param frame massage to send
 * @return AX25_SUCCESS, negative value on error
 */
int iAx25Send (xAx25 *ax25, const xAx25Frame *frame);

/**
 * Read the last AX25 frame received on the channel.
 *
 * @param ax25 AX25 object to operate on.
 * @param frame copy of the last received message
 * @return AX25_SUCCESS, negative value on error
 */
int iAx25Read (xAx25 *ax25, xAx25Frame *frame);

/**
 * Retruns the last error code.
 */
int iAx25Error (xAx25 *ax25);

/**
 *  @}
 * @}
 * @}
 */

/* ========================================================================== */
__END_C_DECLS

#endif /* _SYSIO_AX25_H_ */
