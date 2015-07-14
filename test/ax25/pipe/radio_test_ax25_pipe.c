/**
 * @file test_ax25_serial.c
 * @brief APRS radio demo.
 * 
 * Copyright © 2015 Pascal JEAN aka epsilonRT <pascal.jean--AT--btssn.net>
 * All rights reserved.
 * This software is governed by the CeCILL license <http://www.cecill.info>
 */
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <signal.h>
#include <sys/time.h>

#include <radio/ax25.h>
#include <unistd.h>
#include <fcntl.h>

/* constants ================================================================ */
#define DEVICE "pipe"
#define TRANSMIT_PERIOD 3

#define APRS_MSG ">Test %05d: SolarPi (\x7E) APRS http://www.btssn.net"

/* private variables ======================================================== */
static xAx25 *ax25;
static xAx25Frame *xTxFrame;
static xAx25Frame *xRxFrame;
static int io[2];

/* private functions ======================================================== */

// -----------------------------------------------------------------------------
// Intercepts the CTRL + C signal and closes all properly
static void
vSigIntHandler (int sig) {

  printf("\n%s closed.\nHave a nice day !\n", DEVICE);
  vAx25FrameDelete (xTxFrame);
  vAx25FrameDelete (xRxFrame);
  vAx25Delete (ax25);

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

  iAx25FrameSetInfo (xTxFrame, cInfo, iInfoLen);
  iAx25Send (ax25, xTxFrame);
}

// ------------------------------------------------------------------------------
static void
vSetup (void) {
  int flag;

  // Opens the physical layer
  if (pipe (io) < 0) {

    perror ("pipe");
    exit (EXIT_FAILURE);
  }
  flag = fcntl (io[0], F_GETFL);
  fcntl (io[0], F_SETFL, flag | O_NONBLOCK);

  // Installs the CTRL + C signal handler
  signal(SIGINT, vSigIntHandler);

  // Setup the rx frame
  if ( (xRxFrame = xAx25FrameNew()) == NULL) {

    exit (EXIT_FAILURE);
  }

  // Setup the tx frame
  if ( (xTxFrame = xAx25FrameNew()) == NULL) {

    exit (EXIT_FAILURE);
  }
  iAx25FrameSetDst (xTxFrame, "tlm100", 0);
  iAx25FrameSetSrc (xTxFrame, "nocall", 0);
  iAx25FrameAddRepeater (xTxFrame, "test", 0);
  iAx25FrameSetRepeaterFlag (xTxFrame, 0, true);

  // Initialize the data ax25 layer on the physical layer
  if ( (ax25 = xAx25New()) == NULL) {

    exit (EXIT_FAILURE);
  }
  iAx25SetFdout (ax25, io[1]);
  iAx25SetFdin  (ax25, io[0]);

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

  vSetup ();
  printf ("The ax25 test is in progress on %s.\nPress CTRL+C to quit\n\n", DEVICE);

  for (;;) {

    /*
     * This function will look for new messages from the io channel.
     * It will call the xAx25Read().
     */
    if (bAx25Poll (ax25)) {

      iAx25Read (ax25, xRxFrame);
      iAx25FramePrint (xRxFrame);
      puts("\n");
    }
  }

  return 0;
}
/* ========================================================================== */
