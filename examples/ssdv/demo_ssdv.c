/**
 * @file demo_ssdv.c
 * @brief SSDV radio demo.
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
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include <sys/time.h>

#include <radio/ax25.h>
#include <radio/serial.h>
#include <ssdv/ssdv.h>
#include <SDL/SDL.h>
#include <SDL/SDL_image.h>

/* constants ================================================================ */
#define DEVICE_PIPE   0
#define DEVICE_SERIAL 1

#define DEVICE DEVICE_PIPE
#define SERIAL_BAUDRATE 2400
#define SERIAL_DEVICE "/dev/ttyAMA0"

#define TRANSMIT_PERIOD_S 3UL
#define TRANSMIT_PERIOD_US 0UL

#define SRC_CALLSIGN "nocall"
#define DST_CALLSIGN "tlm100"

#define SSDV_FNAME_IN "in_%d.sdv"
#define SSDV_FNAME_OUT "out_%d.sdv"
#define SSDV_CALLSIGN SRC_CALLSIGN
#define IMG_FNAME_OUT "out_%d.jpg"

#if DEVICE == DEVICE_SERIAL
#define DEVICE_NAME SERIAL_DEVICE
#elif DEVICE == DEVICE_PIPE
#define DEVICE_NAME "pipe"
#else
#error "Unknown DEVICE !"
#endif

/* private variables ======================================================== */
static xAx25 *ax25;
static xAx25Frame *xTxFrame;
static xAx25Frame *xRxFrame;
static int io[2];
static FILE * pxImgIn;
static uint8_t ucTxImageId;
static volatile bool bIsReceived = true;
static clock_t iStart, iEnd;
static SDL_Surface *pxScreen, *pxImage;

/* internal public functions ================================================ */
void vUsage (const char *myname);
void vSetup (int argc, char **argv);
void vSigIntHandler (int sig);
void vSigSendHandler (int sig);
FILE * pxOpenFile (const char * mode, const char * cFormat, int iIndex);
int iSsdvDecode (FILE * pxSsdvFile, FILE * pxJpegFile, char * cCallSign);
int iSsdvEncode (FILE * pxJpegFile, FILE * pxSsdvFile,
                                  const char * cCallSign, uint8_t ucTxImageId);
bool bDiff (FILE * f1, FILE * f2);
FILE * pxMakeSsdvFile (void);

// -----------------------------------------------------------------------------
int
main (int argc, char **argv) {
  int iContinue = 1;
  SDL_Event event;

  printf ("LibRadio Ssdv embedded frame test\n");
  printf ("Copyright (c) 2014 Pascal JEAN <pascal.jean@btssn.net>\n\n");

  vSetup (argc, argv);
  printf ("The ssdv test is in progress on %s.\nPress CTRL+C to quit\n\n", DEVICE_NAME);
  printf ("Transmit period: %.03f sec.\n", (double)TRANSMIT_PERIOD_S + (double)TRANSMIT_PERIOD_US / 1E6);

  while (iContinue) {

    /*
     * This function will look for new messages from the io channel.
     * It will call the xAx25Read().
     */
    if (SDL_PollEvent (&event)) {

      switch (event.type) {

        case SDL_QUIT:
          iContinue = 0;
      }
    }

    if (bAx25Poll (ax25)) {

      iAx25Read (ax25, xRxFrame);
      putchar('+');
      fflush(stdout);

      // Stockage
      if (xRxFrame->info_len == SSDV_PKT_SIZE) {
        int iError;
        // Le paquet a la bonne longueur

        if (ssdv_dec_is_packet (xRxFrame->info, &iError) == 0) {
          ssdv_packet_info_t xPktInfo;
          char cCallSign[7];
          static uint16_t usImgId = -1;
          static FILE * pxSsdvOut;
          // C'est bien un paquet SSDV

          ssdv_dec_header (&xPktInfo, xRxFrame->info);

          if (xPktInfo.image_id != usImgId) {
            char ucBuffer[80];

            // L'Id image a changé
            if (!pxScreen) {

              // Création de la fenêtre de visualisation
              pxScreen = SDL_SetVideoMode (xPktInfo.width, xPktInfo.height, 32, SDL_HWSURFACE);
              if (!pxScreen) {

                fprintf (stderr, "Video initialization failed: %s\n", SDL_GetError());
                exit(EXIT_FAILURE);
              }
            }

            if (pxSsdvOut) {
              FILE * pxImgOut;
              int iError;

              // Traitement image précédente
              iEnd = clock() - iStart;

              rewind (pxSsdvOut);
              printf ("\n------------------ Decode new image %d from %s ------------------\n",
                      usImgId,  ssdv_dec_callsign (cCallSign, xPktInfo.callsign));
              printf ("Time to received: %.03f sec.\n", (double)iEnd /  CLOCKS_PER_SEC);

              pxImgOut = pxOpenFile ("w+b", IMG_FNAME_OUT, usImgId);
              iError = iSsdvDecode (pxSsdvOut, pxImgOut, cCallSign);
              fclose (pxImgOut);

              if (iError >= 0) {
                // Pas d'erreur
                FILE * pxSsdvIn = pxOpenFile ("rb", SSDV_FNAME_IN, usImgId);
                SDL_Rect xOrigin;

                xOrigin.x = 0;
                xOrigin.y = 0;

                snprintf (ucBuffer, sizeof(ucBuffer), IMG_FNAME_OUT, usImgId);
                SDL_FreeSurface (pxImage);
                pxImage = IMG_Load(ucBuffer);
                if(pxImage == NULL) {

                  fprintf(stderr, "IMG_Load: %s\n", IMG_GetError());
                  exit(EXIT_FAILURE);
                }
                SDL_BlitSurface(pxImage, NULL, pxScreen, &xOrigin);
                SDL_WM_SetCaption (ucBuffer, NULL);
                SDL_Flip(pxScreen);

                printf (IMG_FNAME_OUT " file decoded\n", usImgId);

                if (bDiff (pxSsdvIn, pxSsdvOut)) {

                  printf ("Error: SSDV files are different !\n");
                }
                else {

                  printf ("Success: SSDV files are identical.\n");
                }
                fclose (pxSsdvIn);
              }
              else {

                // Erreur décodage
                printf ("Error during " IMG_FNAME_OUT " file decoded: %d\n", usImgId, iError);
              }
              fclose (pxSsdvOut);
              bIsReceived = true;
            }

            // Nouvelle image
            iStart = clock();
            printf ("\n------------------ Receive new image %d from %s ------------------\n",
              xPktInfo.image_id,
              ssdv_dec_callsign (cCallSign, xPktInfo.callsign));
            usImgId = xPktInfo.image_id;
            pxSsdvOut = pxOpenFile ("w+b", SSDV_FNAME_OUT, usImgId);
          }

          fwrite (xRxFrame->info, 1, SSDV_PKT_SIZE, pxSsdvOut);
        }
      }
    }
  }
  vSigIntHandler (SIGQUIT);
  return 0;
}

// -----------------------------------------------------------------------------
FILE *
pxMakeSsdvFile (void) {
  int iError;
  FILE * f;

  f = pxOpenFile ("w+b", SSDV_FNAME_IN, ucTxImageId);
  rewind (pxImgIn);
  printf ("\n------------------ Encode new image %d ------------------\n",
            ucTxImageId);
  iError = iSsdvEncode (pxImgIn, f, SSDV_CALLSIGN, ucTxImageId);
  if (iError != 0) {

    fprintf (stderr, "iSsdvEncode error: %d\n", iError);
    exit (EXIT_FAILURE);
  }
  rewind (f);
  putchar('\n');
  return f;
}

// -----------------------------------------------------------------------------
// Send out message periodically...
void
vSigSendHandler (int sig) {
  char cInfo[SSDV_PKT_SIZE];
  size_t iLen;
  static FILE * pxSsdvFile;

  if (pxSsdvFile == 0) {

    pxSsdvFile = pxMakeSsdvFile ();
  }

  do {

    iLen = fread (cInfo, 1, SSDV_PKT_SIZE, pxSsdvFile);

    if (iLen < SSDV_PKT_SIZE) {

      if (ferror (pxSsdvFile)) {

        perror("Read SsdvFile: ");
        exit (EXIT_FAILURE);
      }
      if (feof (pxSsdvFile)) {

        fclose (pxSsdvFile);
        ucTxImageId++;
        pxSsdvFile = pxMakeSsdvFile ();
      }
      else {

        fprintf(stderr, "Premature end of file\n");
        exit (EXIT_FAILURE);
      }
    }
  } while (iLen != SSDV_PKT_SIZE);

  iAx25FrameSetInfo (xTxFrame, cInfo, iLen);
  iAx25Send (ax25, xTxFrame);
  putchar('.');
  fflush(stdout);
}

// ------------------------------------------------------------------------------
void
vSetup (int argc, char **argv) {

  if (argc == 1) {

    fprintf(stderr, "Error: you must give at least the jpeg file name !\n");
    vUsage(argv[0]);
    exit (EXIT_FAILURE);
  }

#if DEVICE == DEVICE_SERIAL
  if ((io[0] = iSerialOpen (SERIAL_DEVICE, SERIAL_BAUDRATE)) < 0) {

    perror ("iSerialOpen:");
    exit (EXIT_FAILURE);
  }
  io[1] = io[0];

#elif DEVICE == DEVICE_PIPE
  if (pipe (io) < 0) {

    perror ("Pipe: ");
    exit (EXIT_FAILURE);
  }
  int flag = fcntl (io[0], F_GETFL);
  fcntl (io[0], F_SETFL, flag | O_NONBLOCK);
#endif

  if (SDL_Init(SDL_INIT_VIDEO) != 0) {

    fprintf(stderr, "SDL Error : %s\n", SDL_GetError());
    exit(EXIT_FAILURE);
  }

  pxImgIn = fopen (argv[1], "rb");
  if (pxImgIn == NULL) {

    perror("ImgIn: ");
    exit (EXIT_FAILURE);
  }

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

  iAx25FrameSetDst (xTxFrame, DST_CALLSIGN, 0);
  iAx25FrameSetSrc (xTxFrame, SRC_CALLSIGN, 0);

  // Initialize the data ax25 layer on the physical layer
  if ( (ax25 = xAx25New()) == NULL) {

    exit (EXIT_FAILURE);
  }
  iAx25SetFdout (ax25, io[1]);
  iAx25SetFdin  (ax25, io[0]);

  // Setup periodic alarm
  struct itimerval period = {
      { TRANSMIT_PERIOD_S, TRANSMIT_PERIOD_US }, /* 1st signal in [s], [us] */
      { TRANSMIT_PERIOD_S, TRANSMIT_PERIOD_US }  /* period time   [s], [us] */
  };
  signal(SIGALRM, vSigSendHandler);
  setitimer(ITIMER_REAL, &period, NULL); /* start periodic SIGALRM signals */
}

// ------------------------------------------------------------------------------
int
iSsdvDecode (FILE * pxSsdvFile, FILE * pxJpegFile, char * cCallSign) {
  ssdv_t ssdv;
  int i;
  uint8_t image_id = 0, pkt[SSDV_PKT_SIZE],  *jpeg;
  size_t jpeg_length;

  ssdv_dec_init(&ssdv);
  rewind (pxSsdvFile);
  rewind (pxJpegFile);

  jpeg_length = 1024 * 1024 * 4;
  jpeg = malloc (jpeg_length);
  ssdv_dec_set_buffer (&ssdv, jpeg, jpeg_length);

  i = 0;
  while (fread(pkt, 1, SSDV_PKT_SIZE, pxSsdvFile) > 0) {

    /* Test the packet is valid */
    if (ssdv_dec_is_packet (pkt, NULL) != 0)
      continue;

    if (i == 0) {
      ssdv_packet_info_t info;

      ssdv_dec_header (&info, pkt);
      image_id = info.image_id;
      if (cCallSign) {

        ssdv_dec_callsign (cCallSign, info.callsign);
      }
    }
    /* Feed it to the decoder */
    ssdv_dec_feed (&ssdv, pkt);
    i++;
  }

  ssdv_dec_get_jpeg (&ssdv, &jpeg, &jpeg_length);
  fwrite (jpeg, 1, jpeg_length, pxJpegFile);
  free (jpeg);

  sync();
  printf ("Decode %i packets\n", i);
  return image_id;
}

// ------------------------------------------------------------------------------
int
iSsdvEncode (FILE * pxJpegFile, FILE * pxSsdvFile, const char * cCallSign,
                                                          uint8_t ucTxImageId) {
  ssdv_t ssdv;
  uint8_t pkt[SSDV_PKT_SIZE], b[128];
  int i, c;

  ssdv_enc_init (&ssdv, cCallSign, ucTxImageId);
  ssdv_enc_set_buffer (&ssdv, pkt);
  rewind (pxSsdvFile);
  rewind (pxJpegFile);

  i = 0;

  for (;;) {

    while ((c = ssdv_enc_get_packet(&ssdv)) == SSDV_FEED_ME) {
      size_t r;

      r = fread(b, 1, 128, pxJpegFile);

      if (r <= 0) {

        fprintf(stderr, "Premature end of file\n");
        break;
      }
      ssdv_enc_feed(&ssdv, b, r);
    }

    if (c == SSDV_EOI) {

      fprintf(stderr, "ssdv_enc_get_packet said EOI\n");
      break;
    }
    else if (c != SSDV_OK) {

      fprintf(stderr, "ssdv_enc_get_packet failed: %i\n", c);
      return(-1);
    }

    fwrite(pkt, 1, SSDV_PKT_SIZE, pxSsdvFile);
    i++;
  }
  sync();
  printf ("Encode %i packets\n", i);
  return 0;
}


// -----------------------------------------------------------------------------
void
vUsage (const char *myname) {

  printf ("usage: %s FileName\n", myname);
  printf ("  Transmits and receives a jpeg file and  checks that the received file"
          "\nis the same as that which was transmitted.\n");
}

// -----------------------------------------------------------------------------
// Intercepts the CTRL + C signal and closes all properly
void
vSigIntHandler (int sig) {

  printf("\n%s closed.\nHave a nice day !\n", DEVICE_NAME);
  SDL_FreeSurface(pxImage);
  SDL_Quit();
  fclose(pxImgIn);
  vAx25FrameDelete (xTxFrame);
  vAx25FrameDelete (xRxFrame);
  vAx25Delete (ax25);

  exit(EXIT_SUCCESS);
}

// -----------------------------------------------------------------------------
FILE *
pxOpenFile (const char * mode, const char * cFormat, int iIndex) {
  FILE * f;
  char cName[80];

  snprintf (cName, sizeof(cName), cFormat, iIndex);

  f = fopen (cName, mode);
  if (f == NULL) {

    perror(cName);
    exit (EXIT_FAILURE);
  }
  return f;
}

// -----------------------------------------------------------------------------
bool
bDiff (FILE * f1, FILE * f2) {
  uint8_t *p1, *p2;
  int i1, i2;
  bool bIsDiff = false;

  rewind (f1);
  rewind (f2);
  p1 = malloc (1024);
  p2 = malloc (1024);

  for (;;) {

    i1 = fread (p1, 1, 1024, f1);
    i2 = fread (p2, 1, 1024, f2);

    if (ferror(f1) || ferror(f2)) {

      perror ("bDiff: ");
      bIsDiff = true;
      break;
    }

    if (i1 != i2) {

      bIsDiff = true;
      break;
    }

    if (feof (f1))
      break;

    if (memcmp (p1, p2, i1) != 0) {

      bIsDiff = true;
      break;
    }
  }

  free (p1);
  free (p2);
  return bIsDiff;
}

/* ========================================================================== */
