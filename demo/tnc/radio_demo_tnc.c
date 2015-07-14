/**
 * @file demo_tnc.c
 * @brief TNC demo
 * Sends a binary packet to the TNC on the serial bus and waits an acknowledge
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

#include <radio/tnc.h>
#include <sysio/serial.h>
#include <unistd.h>
#include <fcntl.h>

/* constants ================================================================ */
#define SERIAL_BAUDRATE 38400
#define SERIAL_DEVICE "/dev/ttyAMA0"

#define TRANSMIT_PERIOD 3

#define TNC_TXBUFSIZE 256
#define TNC_RXBUFSIZE 16

/* private variables ======================================================== */
static xTnc *tnc;
static uint8_t msg[TNC_TXBUFSIZE];
static int iSerialFd;

/* private functions ======================================================== */

// -----------------------------------------------------------------------------
static void
vUsage (const char *myname) {

  printf ("usage: %s device\n", myname);
}

// -----------------------------------------------------------------------------
// Intercepts the CTRL + C signal and closes all properly
static void
vSigIntHandler (int sig) {

  printf("\n%s closed.\nHave a nice day !\n", SERIAL_DEVICE);
  vTncDelete (tnc);

  exit(EXIT_SUCCESS);
}

// ------------------------------------------------------------------------------
static void
vSetup (int argc, char **argv) {

  if (argc == 1) {

    fprintf(stderr, "Error: you must give at least the device name !\n");
    vUsage(argv[0]);
    exit (EXIT_FAILURE);
  }

  if ((iSerialFd = iSerialOpen (SERIAL_DEVICE, SERIAL_BAUDRATE)) < 0) {

    perror ("iSerialOpen:");
    exit (EXIT_FAILURE);
  }

  // Installs the CTRL + C signal handler
  signal(SIGINT, vSigIntHandler);


  // Initialize the data tnc layer on the physical layer
  if ( (tnc = xTncNew(TNC_RXBUFSIZE)) == NULL) {

    exit (EXIT_FAILURE);
  }
  iTncSetFdout (tnc, iSerialFd);
  iTncSetFdin  (tnc, iSerialFd);

  for (int i = 0; i < TNC_TXBUFSIZE; i++)
    msg[i] = i;
}

/* internal public functions ================================================ */
int
main (int argc, char **argv) {
  int i;

  vSetup (argc, argv);
  printf ("The tnc test is in progress on %s.\nPress CTRL+C to quit\n\n", SERIAL_DEVICE);

  for (;;) {

    i = iTncWrite (tnc, msg, sizeof(msg));
    if (i < 0) {

      fprintf (stderr, "\niTncWrite Error: %d\n", i);
    }
    else {

      printf ("\nSend %d bytes\n", i);
    }
    do {

      i = iTncPoll (tnc);
      if (i == TNC_EOT) {

        printf ("Received %lu bytes: ", tnc->len);
        for (int i = 0; i < tnc->len; i++)
          printf ("%02X ", tnc->rxbuf[i]);
        putchar ('\n');
        break;
      }
    } while (i >= 0);

    if (i < 0) {

      fprintf (stderr, "iTncPoll Error: %d\n", i);
    }
    sleep (TRANSMIT_PERIOD);
  }

  return 0;
}
/* ========================================================================== */
