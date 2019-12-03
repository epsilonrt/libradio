/**
 * @file test_tnc_serial.c
 * @brief APRS radio demo.
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

#include <radio/tnc.h>
#include <unistd.h>
#include <fcntl.h>

/* constants ================================================================ */
#define DEVICE "pipe"
#define TRANSMIT_PERIOD 3
#define TNC_BUFFER_SIZE 256

/* private variables ======================================================== */
static xTnc *tnc;
static uint8_t msg[TNC_BUFFER_SIZE];
static int io[2];

/* private functions ======================================================== */

// -----------------------------------------------------------------------------
// Intercepts the CTRL + C signal and closes all properly
static void
vSigIntHandler (int sig) {

  printf("\n%s closed.\nHave a nice day !\n", DEVICE);
  vTncDelete (tnc);

  exit(EXIT_SUCCESS);
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


  // Initialize the data tnc layer on the physical layer
  if ( (tnc = xTncNew(TNC_BUFFER_SIZE)) == NULL) {

    exit (EXIT_FAILURE);
  }
  iTncSetFdout (tnc, io[1]);
  iTncSetFdin  (tnc, io[0]);
  
  for (int i = 0; i < TNC_BUFFER_SIZE; i++)
    msg[i] = i;
}

/* internal public functions ================================================ */
int
main (int argc, char **argv) {
  int i;

  vSetup ();
  printf ("The tnc test is in progress on %s.\nPress CTRL+C to quit\n\n", DEVICE);

  for (;;) {

    i = iTncWrite (tnc, msg, sizeof(msg));
    if (i < 0) {
      
      fprintf (stderr, "\niTncWrite Error: %d\n", i);
    }
    else {
      
      printf ("\nSend %d bytes\n", i);
    }

    i = iTncPoll (tnc);
    if (i == TNC_EOT) {
      
      printf ("Received %ld bytes: ", tnc->len);
      for (int i = 0; i < tnc->len; i++)
        printf ("%02X ", tnc->rxbuf[i]);
      putchar ('\n');
    }
    else if (i < 0) {
      
      fprintf (stderr, "iTncPoll Error: %d\n", i);
    }
    
    sleep (TRANSMIT_PERIOD);
  }

  return 0;
}
/* ========================================================================== */
