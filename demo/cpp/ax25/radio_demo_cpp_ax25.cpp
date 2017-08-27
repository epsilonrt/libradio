/**
 * @file demo_cpp_ax25.cpp
 * @brief Démonstration Ax25 en C++
 *
 * Usage: demo_cpp_ax25 /dev/ttyAMA0 [baud]
 * - Envoi périodique d'une trame AX25 sur une liaison série
 * - Affichage des trames AX25 sur cette même liaison série
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
#include <sysiopp/serial.hpp>

/* constants ================================================================ */
#define BAUDRATE 2400
#define TRANSMIT_PERIOD 3

#define APRS_MSG ">Test %05d: SolarPi APRS http://www.btssn.net"

/* private variables ======================================================== */
Serial * serial;
Ax25 * ax25;
Ax25Frame * txfrm;

/* private functions ======================================================== */

// -----------------------------------------------------------------------------
// Intercepts the CTRL + C signal and closes all properly
static void
vSigIntHandler (int sig) {

  printf("\nserial port closed.\nHave a nice day !\n");
  serial->close();
  delete serial;
  delete txfrm;
  delete ax25;
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
  txfrm->setInfo (cInfo, iInfoLen);

  printf ("Send Frame: [%s]\n", txfrm->toStr());
  ax25->send (txfrm);
}

// ------------------------------------------------------------------------------
static void
vSetup (const char * cDev, int iBaudrate) {

  serial = new Serial (cDev, iBaudrate);
  // Opens the serial port
  if (!serial->open()) {

    perror ("iSerialOpen:");
    exit (EXIT_FAILURE);
  }

  // Installs the CTRL + C signal handler
  signal(SIGINT, vSigIntHandler);

  
  // Setup the frame
  txfrm = new Ax25Frame;
  txfrm->getDst().setCallsign ("tlm100");
  txfrm->getSrc().setCallsign ("nocall");

  // Initialize the data ax25 layer on the serial port
  ax25 = new Ax25;
  ax25->setFdout (serial->fileno());
  ax25->setFdin  (serial->fileno());

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
    if (ax25->poll()) {
      static Ax25Frame xRxFrame;

      ax25->read (xRxFrame);
      printf ("Recv Frame: [%s]\n\n", xRxFrame.toStr());
    }
  }

  return 0;
}
/* ========================================================================== */
