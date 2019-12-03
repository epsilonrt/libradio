/**
 * @file serial.h
 * @brief Liaison série
 * @author Pascal JEAN <pjean@btssn.net>
 *          @copyright 2014 GNU Lesser General Public License version 3
 *          <http://www.gnu.org/licenses/lgpl.html>
 * @version $Id$
 * Revision History ---
 *    20140208 - Initial version
 */

#ifndef _RADIO_SERIAL_H_
#define _RADIO_SERIAL_H_

#include <radio/defs.h>

/* *INDENT-OFF* */
__BEGIN_C_DECLS
/* ========================================================================== */

/* internal public functions ================================================ */

/**
 *  Open and initialise the serial port
 *
 * @param device device to operate on.
 * @param baud baudrate
 * @return file descriptor on the device, negative value on error
 */
int iSerialOpen (const char *device, const int baud);

/**
 *  Release the serial port
 *
 * @param file descriptor on the device.
 */
void vSerialClose (int fd);

/**
 *  Flush the serial buffers (both tx & rx)
 *
 * @param file descriptor on the device.
 */
void vSerialFlush (int fd);

/* ========================================================================== */
__END_C_DECLS
/* *INDENT-ON* */
#endif /* _RADIO_SERIAL_H_ */
