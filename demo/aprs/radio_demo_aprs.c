/**
 * @file demo_aprs.c
 * @brief APRS radio demo.
 *
 * This example shows how to read and decode APRS radio packets.
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
#include <sysio/serial.h>

/* constants ================================================================ */
#define BAUDRATE 2400
#define TRANSMIT_PERIOD 3

#define APRS_MSG ">Test %05d: SolarPi APRS http://www.btssn.net"

/* private variables ======================================================== */
static xAx25 *ax25;
static xAx25Frame *xTxFrame;
static xAx25Frame *xRxFrame;
static int iSerialFd;

/* private functions ======================================================== */

// -----------------------------------------------------------------------------
// Intercepts the CTRL + C signal and closes all properly
static void
vSigIntHandler (int sig) {

  printf("\nserial port closed.\nHave a nice day !\n");
  vAx25FrameDelete (xTxFrame);
  vAx25FrameDelete (xRxFrame);
  vAx25Delete (ax25);
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
  iAx25FrameSetInfo (xTxFrame, cInfo, iInfoLen);

  printf ("Send Frame: [%s]\n", xAx25FrameToStr (xTxFrame));
  iAx25Send (ax25, xTxFrame);
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

  // Setup the rx frame
  xRxFrame = xAx25FrameNew();

  // Setup the tx frame
  xTxFrame = xAx25FrameNew();
  iAx25FrameSetDst (xTxFrame, "tlm100", 0);
  iAx25FrameSetSrc (xTxFrame, "nocall", 0);
  iAx25FrameAddRepeater (xTxFrame, "test", 0);
  iAx25FrameSetRepeaterFlag (xTxFrame, 0, true);

  // Initialize the data ax25 layer on the physical layer
  ax25 = xAx25New();
  iAx25SetFdout (ax25, iSerialFd);
  iAx25SetFdin (ax25, iSerialFd);

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
    if (bAx25Poll (ax25)) {

      iAx25Read (ax25, xRxFrame);
      printf ("Recv Frame: [%s]\n\n", xAx25FrameToStr (xRxFrame));
    }
  }

  return 0;
}
/* ========================================================================== */
