/**
 * @file serial.c
 * @brief Liaison série (Implémentation)
 * @author Pascal JEAN <pjean@btssn.net>
 *          @copyright 2014 GNU Lesser General Public License version 3
 *          <http://www.gnu.org/licenses/lgpl.html>
 * @version $Id$
 * Revision History ---
 *    20140208 - Initial version
 */
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <sys/ioctl.h>

#include "radio/serial.h"
/* ========================================================================== */

/* internal public functions ================================================ */

// -----------------------------------------------------------------------------
int
iSerialOpen (const char *device, const int baud) {
  struct termios options;
  speed_t myBaud;
  int     status, fd;

  switch (baud) {
    case     50:
      myBaud =     B50;
      break;
    case     75:
      myBaud =     B75;
      break;
    case    110:
      myBaud =    B110;
      break;
    case    134:
      myBaud =    B134;
      break;
    case    150:
      myBaud =    B150;
      break;
    case    200:
      myBaud =    B200;
      break;
    case    300:
      myBaud =    B300;
      break;
    case    600:
      myBaud =    B600;
      break;
    case   1200:
      myBaud =   B1200;
      break;
    case   1800:
      myBaud =   B1800;
      break;
    case   2400:
      myBaud =   B2400;
      break;
    case   4800:
      myBaud =   B4800;
      break;
    case   9600:
      myBaud =   B9600;
      break;
    case  19200:
      myBaud =  B19200;
      break;
    case  38400:
      myBaud =  B38400;
      break;
    case  57600:
      myBaud =  B57600;
      break;
    case 115200:
      myBaud = B115200;
      break;
    case 230400:
      myBaud = B230400;
      break;

    default:
      return -2;
  }

  if ((fd = open (device, O_RDWR | O_NOCTTY | O_NDELAY)) == -1)
    return -1;

  fcntl (fd, F_SETFL, O_RDWR);

// Get and modify current options:

  tcgetattr (fd, &options);

  cfmakeraw   (&options);
  cfsetispeed (&options, myBaud);
  cfsetospeed (&options, myBaud);

  options.c_cflag |= (CLOCAL | CREAD);
  options.c_cflag &= ~PARENB;
  options.c_cflag &= ~CSTOPB;
  options.c_cflag &= ~CSIZE;
  options.c_cflag |= CS8;
  options.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
  options.c_oflag &= ~OPOST;

  // read() should never block
  options.c_cc [VMIN]  = 0;
  options.c_cc [VTIME] = 0;

  tcflush (fd, TCIOFLUSH);
  tcsetattr (fd, TCSANOW, &options);

  ioctl (fd, TIOCMGET, &status);
  status |= TIOCM_DTR;
  status |= TIOCM_RTS;
  ioctl (fd, TIOCMSET, &status);

  usleep (10000);  // 10mS

  return fd;
}

// -----------------------------------------------------------------------------
void
vSerialFlush (int fd) {

  tcflush (fd, TCIOFLUSH);
}

// -----------------------------------------------------------------------------
void
vSerialClose (int fd) {

  close (fd);
}

/* ========================================================================== */
