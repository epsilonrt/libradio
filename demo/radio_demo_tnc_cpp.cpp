/**
 * @file demo/cpp/tnc/demo_cpp_tnc.cpp
 * @brief Démonstration TNC en C++
 *
 * Usage: demo_cpp_tnc /dev/ttyAMA0 [baud]
 * - Envoi périodique d'une trame APRS vers un TNC sur une liaison série
 * - Affichage des trames APRS/SSDV reçues par le TNC
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
#include <radio/tnc.hpp>
#include <sysio/serial.hpp>

/* constants ================================================================ */
#define BAUDRATE 38400
#define TRANSMIT_PERIOD 3
#define APRS_MSG ">Test %05d: SolarPi APRS http://www.btssn.net"

/* global variables ========================================================= */
Serial * serial;
Tnc * tnc;
bool bTxEnable = true;

/* private functions ======================================================== */
// -----------------------------------------------------------------------------
// Intercepts the CTRL + C signal and closes all properly
static void
vSigIntHandler (int sig) {

  serial->close();
  delete tnc;
  delete serial;
  printf("\nserial port closed.\nHave a nice day !\n");
  exit(EXIT_SUCCESS);
}

// -----------------------------------------------------------------------------
// Send out message periodically...
static void
vSigSendHandler (int sig) {
  static uint16_t usFrameCounter = 0;
  char cInfo[80];

  snprintf (cInfo, 80, APRS_MSG, ++usFrameCounter);

  printf ("Send Frame: [%s]\n", cInfo);
  tnc->write (cInfo);
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

  tnc = new Tnc(256);
  // Initialize the data tnc layer on the serial port
  tnc->setFdout (serial->fileno());
  tnc->setFdin  (serial->fileno());
  
  // Installs the CTRL + C signal handler
  signal(SIGINT, vSigIntHandler);

  if (bTxEnable) {
    // Setup periodic alarm
    struct itimerval period = {
        { TRANSMIT_PERIOD, 0, }, /* 1st signal in [s], [us] */
        { TRANSMIT_PERIOD, 0, }, /* period time   [s], [us] */
    };
    signal(SIGALRM, vSigSendHandler);
    setitimer(ITIMER_REAL, &period, NULL); /* start periodic SIGALRM signals */
  }
}
/* internal public functions ================================================ */
int
main (int argc, char **argv) {
  int iBaudrate = BAUDRATE;
  const char * cDev = "";
  int iIndex = 1;

  if (argc < 2) {

    printf("Usage: %s /dev/ttyAMA0 [baud] [-n]\n", argv[0]);
    exit(EXIT_FAILURE);
  }
  // Paramètre 1:
  if (strcmp (argv[iIndex], "-n") == 0)
    bTxEnable = false;
  else
    cDev = argv[iIndex];
  iIndex++;

  // Paramètre 2:
  if (argc > iIndex) {

    if (bTxEnable)
      iBaudrate = atoi(argv[iIndex]);
    else
      cDev = argv[iIndex];
  }
  iIndex++;

  if (argc > iIndex) {

    if (bTxEnable) {
      if (strcmp (argv[iIndex], "-n") == 0)
        bTxEnable = false;
    }
    else
      iBaudrate = atoi(argv[iIndex]);
  }

  vSetup (cDev, iBaudrate);
  printf ("The tnc test is in progress on %s (baud=%d).\nTansmitting %s\nPress CTRL+C to quit\n\n", cDev, iBaudrate, (bTxEnable ? "enable" : "disable"));

  for (;;) {
    int iPacketType;

    iPacketType = tnc->poll();
    switch (iPacketType) {

      case Tnc::APRS_PACKET:
        printf ("APRS: [%s]\n", tnc->read());
        break;
      case Tnc::SSDV_PACKET:
        printf ("SSDV: %d-%d\n", tnc->getSsdvImageId(), tnc->getSsdvPacketId());
        break;
      default:
        break;
    }
  }
  return 0;
}

/* ========================================================================== */
