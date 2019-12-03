/**
 * @file demo_aprs.c
 * @brief APRS radio demo.
 *
 * This example shows how to read and decode APRS radio packets.
 * @author Pascal JEAN <pjean@btssn.net>
 *          @copyright 2014 GNU Lesser General Public License version 3
 *          <http://www.gnu.org/licenses/lgpl.html>
 * @version $Id$
 * Revision History ---
 *    20120519 - Initial version
 */
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <signal.h>
#include <sys/time.h>

#include <radio/ax25.hpp>
#include <radio/serial.h>

/* constants ================================================================ */
#define BAUDRATE 2400
#define TRANSMIT_PERIOD 3

#define APRS_MSG ">Test %05d: SolarPi APRS http://www.btssn.net"

/* private variables ======================================================== */
static Ax25 ax25;
static Ax25Frame xTxFrame;
static int iSerialFd;

/* private functions ======================================================== */

// -----------------------------------------------------------------------------
// Intercepts the CTRL + C signal and closes all properly
static void
vSigIntHandler (int sig) {

  printf("\nserial port closed.\nHave a nice day !\n");
  vSerialClose (iSerialFd);
  exit(EXIT_SUCCESS);
}

// -----------------------------------------------------------------------------
// Send out message periodically...
static void
vSigSendHandler (int sig) {
  static uint16_t usFrameCounter = 0;
  char cInfo[80];
  size_t iInfoLen;

  snprintf (cInfo, 80, APRS_MSG, ++usFrameCounter);
  iInfoLen = strlen(cInfo) + 1;
  xTxFrame.setInfo (cInfo, iInfoLen);

  printf ("Send Frame: [%s]\n", xTxFrame.toStr());
  ax25.send (xTxFrame);
}

// ------------------------------------------------------------------------------
static void
vSetup (const char * cDev, int iBaudrate) {

  // Opens the serial port
  if ((iSerialFd = iSerialOpen (cDev, iBaudrate)) < 0) {

    perror ("iSerialOpen:");
    exit (EXIT_FAILURE);
  }

  // Installs the CTRL + C signal handler
  signal(SIGINT, vSigIntHandler);

  // Setup the frame
  xTxFrame.getDst().setCallsign ("tlm100");
  xTxFrame.getSrc().setCallsign ("nocall");

  // Initialize the data ax25 layer on the serial port
  ax25.setFdout (iSerialFd);
  ax25.setFdin  (iSerialFd);

  // Setup periodic alarm
  struct itimerval period = {
      { TRANSMIT_PERIOD, 0, }, /* 1st signal in [s], [us] */
      { TRANSMIT_PERIOD, 0, }, /* period time   [s], [us] */
  };
  signal(SIGALRM, vSigSendHandler);
  setitimer(ITIMER_REAL, &period, NULL); /* start periodic SIGALRM signals */
}

/* internal public functions ================================================ */
int
main (int argc, char **argv) {
  int iBaudrate = BAUDRATE;
  const char * cDev;

  if (argc < 2) {

    printf("Usage: %s /dev/ttyAMA0 [baud]\n", argv[0]);
    exit(EXIT_FAILURE);
  }
  cDev = argv[1];
  if (argc > 2) {

    iBaudrate = atoi(argv[2]);
  }

  vSetup (cDev, iBaudrate);
  printf ("The ax25 test is in progress on %s (baud=%d).\nPress CTRL+C to quit\n\n", cDev, iBaudrate);

  for (;;) {

    /*
     * This function will look for new messages from the io channel.
     * It will call the xAx25Read().
     */
    if (ax25.poll()) {
      static Ax25Frame xRxFrame;

      ax25.read (xRxFrame);
      printf ("Recv Frame: [%s]\n\n", xRxFrame.toStr());
    }
  }

  return 0;
}
/* ========================================================================== */
