/**
 * @file test_ax25_cpp.cpp
 * @brief APRS radio demo.
 *
 * This example shows how to read and decode APRS radio packets.
 * 
 * Copyright © 2015 epsilonRT, All rights reserved.
 * This software is governed by the CeCILL license <http://www.cecill.info>
 */
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <signal.h>
#include <sys/time.h>

#include <radio/ax25.hpp>
#include <sysio/serial.h>
#include <iostream>

using namespace std;

/* constants ================================================================ */
#define BAUDRATE 2400
#define DEVICE "/dev/ttyUSB0"
#define TRANSMIT_PERIOD 3

#define APRS_MSG ">Test %05d: SolarPi (\x7E) APRS http://www.btssn.net"

/* private variables ======================================================== */
static Ax25 ax25;
static Ax25Frame xTxFrame;
static int iSerialFd;

/* private functions ======================================================== */

// -----------------------------------------------------------------------------
// Intercepts the CTRL + C signal and closes all properly
static void
vSigIntHandler (int sig) {

  printf("\n%s closed.\nHave a nice day !\n", DEVICE);
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
  printf ("\nSend frame #%05u (size %ld)\n", usFrameCounter, iInfoLen);

  xTxFrame.setInfo (cInfo, iInfoLen);
  ax25.send (xTxFrame);
}

// ------------------------------------------------------------------------------
static void
vSetup (int iBaudrate) {

  // Opens the serial port
  if ((iSerialFd = iSerialOpen (DEVICE, iBaudrate)) < 0) {

    perror ("iSerialOpen:");
    exit (EXIT_FAILURE);
  }

  // Installs the CTRL + C signal handler
  signal(SIGINT, vSigIntHandler);

  // Setup the frame
  xTxFrame.getDst().setCallsign ("tlm100");
  xTxFrame.getSrc().setCallsign ("nocall");
  xTxFrame.addRepeater ("repok1");

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

  vSetup (iBaudrate);
  printf ("The ax25 test is in progress on %s (baud=%d).\nPress CTRL+C to quit\n", DEVICE, iBaudrate);

  for (;;) {

    /*
     * This function will look for new messages from the io channel.
     * It will call the xAx25Read().
     */
    if (ax25.poll()) {
      static Ax25Frame xRxFrame;

      ax25.read (xRxFrame);
      cout << xRxFrame.toStr() << endl << endl;
    }
  }

  return 0;
}
/* ========================================================================== */
