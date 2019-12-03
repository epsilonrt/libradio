/**
 * @file serial.hpp
 * @brief Serial port (C++).
 * @author Pascal JEAN <pjean@btssn.net>
 *          @copyright 2014 GNU Lesser General Public License version 3
 *          <http://www.gnu.org/licenses/lgpl.html>
 * @version $Id$
 * Revision History ---
 *    20140406 - Initial version
 */

#ifndef _RADIO_SERIAL_HPP_
#define _RADIO_SERIAL_HPP_

/* *INDENT-OFF* */
/* ========================================================================== */
#include <radio/defs.h>

//##############################################################################
//#                                                                            #
//#                            xSerial Class                                   #
//#                                                                            #
//##############################################################################
/**
 * Serial Port Class
 */
class Serial {

public:
  Serial();
  ~Serial();
  int open (const char *device, const int baud);
  void close();
  int fileno() const;
  void flush();

private:
  int fd;
};

/* ========================================================================== */
/* *INDENT-ON* */
#endif /* _RADIO_SERIAL_HPP_ */
