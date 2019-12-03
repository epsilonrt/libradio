/**
 * @file serial.cpp
 * @brief Serial Port (Wrapper C++).
 * @author Pascal JEAN <pjean@btssn.net>
 *          @copyright 2014 GNU Lesser General Public License version 3
 *          <http://www.gnu.org/licenses/lgpl.html>
 * @version $Id$
 * Revision History ---
 *    20140406 - Initial version
 */
#include <radio/serial.hpp>
#include <radio/serial.h>
#include <stdlib.h>
#include <errno.h>
#include <stdexcept>

//##############################################################################
//#                                                                            #
//#                            xSerial Class                                   #
//#                                                                            #
//##############################################################################

/* public  ================================================================== */

// -----------------------------------------------------------------------------
Serial::Serial() : fd(-1) {

}

// -----------------------------------------------------------------------------
Serial::~Serial () {

  close();
}

// -----------------------------------------------------------------------------
int
Serial::open (const char *device, const int baud) {

  fd = iSerialOpen (device, baud);
  return fd;
}

// -----------------------------------------------------------------------------
void Serial::close() {

  if (fd >= 0) {

    vSerialClose (fd);
    fd = -1;
  }
}

// -----------------------------------------------------------------------------
int Serial::fileno() const {

  return fd;
}

// -----------------------------------------------------------------------------
void
Serial::flush() {

  if (fd >= 0) {

    vSerialFlush (fd);
  }
}

/* ========================================================================== */
