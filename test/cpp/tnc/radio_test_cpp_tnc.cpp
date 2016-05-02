/**
 * @file test_tnc_cpp.cpp
 * @brief APRS sysio demo.
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

#include <radio/tnc.h>
#include <unistd.h>
#include <fcntl.h>

#include <radio/tnc.hpp>
#include <iostream>

using namespace std;

/* constants ================================================================ */
#define DEVICE "pipe"
#define TRANSMIT_PERIOD 3
#define TNC_BUFFER_SIZE 256

/* private variables ======================================================== */
static Tnc tnc (TNC_BUFFER_SIZE);
static uint8_t msg[TNC_BUFFER_SIZE];
static int io[2];

/* private functions ======================================================== */

// -----------------------------------------------------------------------------
// Intercepts the CTRL + C signal and closes all properly
static void
vSigIntHandler (int sig) {

  printf ("\n%s closed.\nHave a nice day !\n", DEVICE);

  exit (EXIT_SUCCESS);
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
  signal (SIGINT, vSigIntHandler);

  tnc.setFdout (io[1]);
  tnc.setFdin (io[0]);

  for (int i = 0; i < TNC_BUFFER_SIZE; i++) {
    msg[i] = i;
  }
}

/* internal public functions ================================================ */
int
main (int argc, char **argv) {
  int i;

  vSetup ();
  printf ("The tnc test is in progress on %s.\nPress CTRL+C to quit\n\n", DEVICE);

  for (;;) {

    i = tnc.write (msg, sizeof (msg));
    if (i < 0) {

      fprintf (stderr, "\niTncWrite Error: %d\n", i);
    }
    else {

      printf ("\nSend %d bytes\n", i);
    }

    if (tnc.poll()) {

      printf ("Received %d bytes: ", tnc.getRxlen());
      for (int i = 0; i < tnc.getRxlen(); i++) {
        printf ("%02X ", tnc.read() [i]);
      }
      putchar ('\n');
    }

    if (tnc.getError() < 0) {

      fprintf (stderr, "Tnc Error: %d\n", i);
    }

    sleep (TRANSMIT_PERIOD);
  }

  return 0;
}
/* ========================================================================== */
