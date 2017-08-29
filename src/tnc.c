/**
 * @file src/tnc.c
 * @brief Terminal Node Controller (Implémentation C)
 *
 * Copyright © 2015 epsilonRT, All rights reserved.
 * This software is governed by the CeCILL license <http://www.cecill.info>
 */
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <limits.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <termios.h>

#include <radio/tnc.h>
#include <radio/crc.h>

//##############################################################################
//#                                                                            #
//#                        Errors management                                   #
//#                                                                            #
//##############################################################################

static const char * cErrorMsg[] = {

  "CRC error\n",
  "Not enough memory\n",
  "Object not found\n",
  "Illegal message\n",
  "File not found\n",
  "Io error\n"
};

// -----------------------------------------------------------------------------
static const char *
prvcMsg (int iError) {
  int iIndex = ABS (iError) - 1;

  if ( (iIndex > 0) && (iIndex < COUNTOF (cErrorMsg))) {
    return cErrorMsg[iIndex];
  }
  return "";
}

// -----------------------------------------------------------------------------
static int
prviError (int iError) {

  if (iError < TNC_SUCCESS) {

    fprintf (stderr, "%s", prvcMsg (iError));
  }
  return iError;
}

//##############################################################################
//#                                                                            #
//#                             xTnc Class                                    #
//#                                                                            #
//##############################################################################

/* private ================================================================== */

// -----------------------------------------------------------------------------
static int
prviSetError (xTnc *p, int iError) {

  p->state = iError;
  return prviError (iError);
}

// -----------------------------------------------------------------------------
static int
prvxOpenFile (int fd, int mode) {
  int flag;

  if ( (flag = fcntl (fd, F_GETFL)) != -1) {

    flag &= O_ACCMODE;
    if ( (flag != mode) && (flag != O_RDWR)) {

      fd = -1;
    }
  }
  else {
    fd = -1;
  }
  return fd;
}

// -----------------------------------------------------------------------------
static
uint8_t
htoi (uint8_t c) {

  c = toupper (c);
  if (!isdigit (c)) {
    c -= 7;
  }
  return c - '0';
}


/* public  ================================================================== */

// -----------------------------------------------------------------------------
xTnc *
xTncNew (size_t iRxBufferSize) {
  xTnc *p;

  p = malloc (sizeof (xTnc));
  if (p) {

    iTncClear (p);
    p->rxbuf = malloc (iRxBufferSize);
    if (!p->rxbuf) {

      free (p);
      p = 0;
    }
  }
  return p;
}

// -----------------------------------------------------------------------------
void
vTncDelete (xTnc *p) {

  free (p->rxbuf);
  free (p);
}

// -----------------------------------------------------------------------------
int iTncClear (xTnc *p) {

  if (!p) {

    return prviError (TNC_OBJECT_NOT_FOUND);
  }
  memset (p, 0, sizeof (xTnc));
  p->crc_rx  = CRC_CCITT_INIT_VAL;
  p->fin = -1;
  p->fout = -1;

  return TNC_SUCCESS;
}

// -----------------------------------------------------------------------------
int
iTncError (xTnc *p) {

  if (p) {

    return p->state;
  }
  (void) prviError (TNC_OBJECT_NOT_FOUND);
  return INT_MIN;
}

// -----------------------------------------------------------------------------
int
iTncSetFdin (xTnc *p, int fd) {

  if (!p) {

    return prviSetError (p, TNC_OBJECT_NOT_FOUND);
  }
  p->fin = prvxOpenFile (fd, O_RDONLY);

  return prviSetError (p, TNC_SUCCESS);
}

// -----------------------------------------------------------------------------
int
iTncSetFdout (xTnc *p, int fd) {

  if (!p) {

    return prviSetError (p, TNC_OBJECT_NOT_FOUND);
  }
  p->fout = prvxOpenFile (fd, O_WRONLY);

  return prviSetError (p, TNC_SUCCESS);
}

// -----------------------------------------------------------------------------
int
iTncPoll (xTnc *p) {

  if (!p) {

    return prviSetError (p, TNC_OBJECT_NOT_FOUND);
  }
  else {

    if (p->state == TNC_EOT) {
      p->state = 0;
    }

    if (p->fin < 0) {

      return prviSetError (p, TNC_FILE_NOT_FOUND);
    }
    else {
      ssize_t count;

      do {
        uint8_t c;

        count = read (p->fin, &c, 1);
        if (count < 0) {

          perror ("read: ");
          return prviSetError (p, TNC_IO_ERROR);
        }
        if (count > 0) {

          switch (c) {

            case TNC_SOH:
              p->crc_rx = CRC_CCITT_INIT_VAL;
              p->state = TNC_SOH;
              p->len = 0;
              break;

            case TNC_STX:
              if (p->state == TNC_SOH) {

                p->cnt = 0;
                p->state = TNC_STX;
              }
              else {
                p->state = 0;
              }
              break;

            case TNC_ETX:
              if (p->state == TNC_STX) {

                p->cnt = 0;
                p->crc_tx = 0;
                p->state = TNC_ETX;
              }
              else {
                p->state = 0;
              }
              break;

            case TNC_EOT:
              if (p->state == TNC_ETX) {

                p->state = TNC_EOT;
                if (p->crc_rx != p->crc_tx) {
                  return prviSetError (p, TNC_CRC_ERROR);
                }
              }
              else {
                p->state = 0;
              }
              break;

            default:
              if (isxdigit (c)) {

                // Digit hexa
                switch (p->state) {

                  case TNC_STX:
                    p->crc_rx = usCrcCcittUpdate (c, p->crc_rx);
                    if (p->cnt++ & 1) {
                      // LSB
                      p->rxbuf[p->len++] = p->msb + htoi (c);
                    }
                    else {
                      // MSB
                      p->msb = htoi (c) << 4;
                    }
                    break;

                  case TNC_ETX:
                    if (p->cnt <= 12) {

                      p->crc_tx += ( (uint16_t) htoi (c)) << (12 - p->cnt);
                      p->cnt += 4;
                    }
                    else {

                      // Plus de 4 octets de CRC reçu
                      p->state = TNC_ILLEGAL_MSG;
                    }
                    break;

                  default:
                    // Digit hexa en dehors d'une trame, on ignore
                    p->state = 0;
                    break;
                }
              }
              break;

          }
        }
      }
      while ( (count > 0) && (p->state != TNC_EOT));
    }
  }
  return p->state;
}

// -----------------------------------------------------------------------------
ssize_t
iTncWrite (xTnc *p, const void *buf, size_t count) {
  ssize_t index = 0;


  if (!p) {

    return prviSetError (p, TNC_OBJECT_NOT_FOUND);
  }

  if (p->fout < 0) {

    return prviSetError (p, TNC_FILE_NOT_FOUND);
  }
  else {
    int state = TNC_SOH;
    ssize_t i;
    uint8_t cnt = 0;
    char str[5];
    const uint8_t *b = (uint8_t *) buf;
    uint16_t crc = CRC_CCITT_INIT_VAL;

    tcflush (p->fout, TCIOFLUSH);

    while (state != TNC_EOT) {
      uint8_t c;

      switch (state) {

        case TNC_SOH:
          c = state;
          state = TNC_STX;
          break;

        case TNC_STX:
          c = state;
          state = TNC_TXT;
          break;

        case TNC_TXT:
          if (count) {
            if (cnt++ & 1) {

              // LSB
              c = str[1];
              if (++index >= count) {
                state = TNC_ETX;
              }
            }
            else {

              // MSB
              snprintf (str, 3, "%02X", b[index]);
              c = str[0];
            }
            crc = usCrcCcittUpdate (c, crc);
          }
          else {

            state = TNC_ETX;
            continue;
          }

          break;

        case TNC_ETX:
          c = state;
          cnt = 0;
          state = TNC_CRC;
          snprintf (str, 5, "%04X", crc);
          break;

        case TNC_CRC:
          if (cnt < 4) {

            c = str[cnt++];
          }
          else {

            c = TNC_EOT;
            state = TNC_EOT;
          }
          break;

        default:
          break;

      }
      do {

        i = write (p->fout, &c, 1);
        if (i < 0) {

          perror ("write: ");
          return prviSetError (p, TNC_IO_ERROR);
        }
      }
      while (i == 0);
    }
  }
  return index;
}

/* ========================================================================== */
