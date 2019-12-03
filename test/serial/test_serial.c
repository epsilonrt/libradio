/* Copyright © 2014 Pascal JEAN. All rights reserved.
 *  $Id$ */
/* ========================================================================== */
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <signal.h>

#include <radio/serial.h>

/* constants ================================================================ */
#define BAUDRATE 9600
#define DEVICE "/dev/ttyUSB0"

/* private variables ======================================================== */
static int iSerialFd;
static FILE * xSerialPort;

/* private functions ======================================================== */
// -----------------------------------------------------------------------------
static void
vSigIntHandler (int sig) {

  fprintf (xSerialPort, "\n%s is being closed...\n", DEVICE);
  fclose (xSerialPort);
  printf("\n%s closed.\nHave a nice day !\n", DEVICE);
  exit(EXIT_SUCCESS);
}

// ------------------------------------------------------------------------------
static void
vSetup (int iBaudrate) {

  if ((iSerialFd = iSerialOpen (DEVICE, iBaudrate)) < 0) {

    perror ("serialOpen");
    exit (EXIT_FAILURE);
  }

  if ((xSerialPort = fdopen (iSerialFd, "w+")) == NULL) {

    perror ("fdopen");
    exit (EXIT_FAILURE);
  }

  // vSigIntHandler() intercepte le CTRL+C
  signal(SIGINT, vSigIntHandler);
}

/* internal public functions ================================================ */
int
main (int argc, char **argv) {
  int c;
  int iBaudrate = BAUDRATE;

  if (argc > 1) {

    int iBaud = atoi(argv[1]);
    switch (iBaud) {
      case     50: break;
      case     75: break;
      case    110: break;
      case    134: break;
      case    150: break;
      case    200: break;
      case    300: break;
      case    600: break;
      case   1200: break;
      case   1800: break;
      case   2400: break;
      case   4800: break;
      case   9600: break;
      case  19200: break;
      case  38400: break;
      case  57600: break;
      case 115200: break;
      case 230400: break;

      default:
        fprintf (stderr, "Illegal baudrate: %s\n", argv[1]);
        exit (EXIT_FAILURE);
    }
    iBaudrate = iBaud;
  }

  vSetup (iBaudrate);
  printf ("The serial link test is in progress on %s (baud=%d).\nPress CTRL+C to quit\n", DEVICE, iBaudrate);
  fprintf (xSerialPort, "Serial Test on %s\nPress any key...\n", DEVICE);

  for (;;) {

    c = fgetc (xSerialPort);
    if (c != EOF) {

      fputc (c, xSerialPort);
      putchar (c);
    }
  }

  return 0;
}
