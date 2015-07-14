/**
 * @file sysio/src/ax25.c
 * @brief Couche liaison du protocole AX.25 (Implémentation C)
 *
 * Copyright © 2015 Pascal JEAN aka epsilonRT <pascal.jean--AT--btssn.net>
 * All rights reserved.
 * This software is governed by the CeCILL license <http://www.cecill.info>
 */
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <limits.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

#include <radio/ax25.h>
#include <radio/crc.h>
#include <sysio/log.h>

/*
 * Enable access to the physical layer using streams
 * This option is not yet functional !
 */
#define AX25_CFG_USE_STREAM 0

#if AX25_MAX_RPT > 8
#error "AX25_MAX_RPT must be less than equal to 8."
#endif

#if AX25_CFG_USE_STREAM != 0
#undef AX25_CFG_USE_STREAM
#define AX25_CFG_USE_STREAM 0
#warning "AX25_CFG_USE_STREAM is not yet functional and has been disabled !"
#endif

#define NODE_STRLEN  (AX25_CALL_LEN + 4)
#define FRAME_STRLEN(_r,_l) ((NODE_STRLEN)*((_r)+2)+(_l)+3)

//##############################################################################
//#                                                                            #
//#                        Errors management                                   #
//#                                                                            #
//##############################################################################
static const char * cErrorMsg[] = {

  "File access error\n",  // AX25_FILE_ACCESS_ERROR        = -1
  "Illegal callsign\n",   // AX25_ILLEGAL_CALLSIGN         = -2
  "Invalid frame\n",      // AX25_ILLEGAL_PATH             = -3
  "CRC error\n",          // AX25_CRC_ERROR                = -4
  "Not enough memory\n",  // AX25_NOT_ENOUGH_MEMORY        = -5
  "Not enough repeater\n",// AX25_NOT_ENOUGH_REPEATER      = -6
  "Object not found\n",   // AX25_OBJECT_NOT_FOUND         = -7
  "File not found\n",     // AX25_FILE_NOT_FOUND           = -8
  "Illegal repeater\n",   // AX25_ILLEGAL_REPEATER         = -9
  "No frame received\n"   // AX25_NO_FRAME_RECEIVED        = -10
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

  if (iError < AX25_SUCCESS) {

    PERROR ("%s", prvcMsg (iError));
  }
  return iError;
}

//##############################################################################
//#                                                                            #
//#                          xAx25Node Class                                   #
//#                                                                            #
//##############################################################################

/* private ================================================================== */

/* public  ================================================================== */

// -----------------------------------------------------------------------------
xAx25Node *
xAx25NodeNew (void) {
  xAx25Node *n;

  n = malloc (sizeof (xAx25Node));
  if (n) {
    iAx25NodeClear (n);
  }
  return n;
}

// -----------------------------------------------------------------------------
void
vAx25NodeDelete (xAx25Node *n) {

  free (n);
}

// -----------------------------------------------------------------------------
int
iAx25NodeClear (xAx25Node *p) {

  if (!p) {

    return prviError (AX25_OBJECT_NOT_FOUND);
  }
  memset (p, 0, sizeof (xAx25Node));
  return AX25_SUCCESS;
}

// -----------------------------------------------------------------------------
int
iAx25NodeSetCallsign (xAx25Node *p, const char * callsign) {

  if (!p) {

    return prviError (AX25_OBJECT_NOT_FOUND);
  }
  strncpy (p->callsign, callsign, AX25_CALL_LEN);
  p->callsign[AX25_CALL_LEN] = 0; // prevents core dump
  return AX25_SUCCESS;
}

// -----------------------------------------------------------------------------
char *
xAx25NodeToStr (const xAx25Node *n) {
  char *str = NULL;

  if (n) {

    str = malloc (NODE_STRLEN);
    if (str) {
      char *p = str;

      for (int i = 0; (i < AX25_CALL_LEN) && (n->callsign[i]); i++) {

        *p++ = n->callsign[i];
      }
      if (n->ssid) {

        p += sprintf (p, "-%d", n->ssid);
      }
      if (n->flag) {

        *p++ = '*';
      }
      *p = 0;
    }
  }
  else {
    prviError (AX25_OBJECT_NOT_FOUND);
  }

  return str;
}

// -----------------------------------------------------------------------------
int
iAx25NodeFilePrint (const xAx25Node *p, FILE * f) {

  if (p) {

    if (f) {
      char *str = xAx25NodeToStr (p);

      if (str) {

        fprintf (f, "%s", str);
        free (str);
        return AX25_SUCCESS;
      }
      return prviError (AX25_NOT_ENOUGH_MEMORY);
    }
    return prviError (AX25_FILE_NOT_FOUND);
  }
  return prviError (AX25_OBJECT_NOT_FOUND);
}

// -----------------------------------------------------------------------------
int
iAx25NodePrint (const xAx25Node *p) {

  return iAx25NodeFilePrint (p, stdout);
}

//##############################################################################
//#                                                                            #
//#                         xAx25Frame Class                                   #
//#                                                                            #
//##############################################################################

/* private ================================================================== */


/* public  ================================================================== */

// -----------------------------------------------------------------------------
xAx25Frame *
xAx25FrameNew (void) {
  xAx25Frame *p;

  p = malloc (sizeof (xAx25Frame));
  if (p) {

    p->src = xAx25NodeNew();
    p->dst = xAx25NodeNew();
    memset (p->repeaters, 0, sizeof (xAx25Node *) * AX25_MAX_RPT);
    p->repeaters_len = 0;
    iAx25FrameClear (p);
  }
  return p;
}

// -----------------------------------------------------------------------------
void
vAx25FrameDelete (xAx25Frame *p) {
  if (p) {

    vAx25NodeDelete (p->src);
    vAx25NodeDelete (p->dst);
    for (int i = 0; i < p->repeaters_len; i++) {
      vAx25NodeDelete (p->repeaters[i]);
    }
  }
  free (p);
}

// -----------------------------------------------------------------------------
int
iAx25FrameClear (xAx25Frame *p) {

  if (!p) {

    return prviError (AX25_OBJECT_NOT_FOUND);
  }
  p->ctrl = AX25_CTRL_UI;
  p->pid = AX25_PID_NOLAYER3;
  memset (p->info, 0, AX25_FRAME_BUF_LEN);
  p->info_len = 0;
  iAx25NodeClear (p->src);
  iAx25NodeClear (p->dst);
  for (int i = 0; i < p->repeaters_len; i++) {

    vAx25NodeDelete (p->repeaters[i]);
    p->repeaters[i] = 0;
  }
  p->repeaters_len = 0;

  return AX25_SUCCESS;
}

// -----------------------------------------------------------------------------
int
iAx25FrameSetSrc (xAx25Frame *p, const char *callsign, uint8_t ssid) {

  if (!p) {

    return prviError (AX25_OBJECT_NOT_FOUND);
  }
  iAx25NodeSetCallsign (p->src, callsign);
  p->src->ssid = ssid;
  return AX25_SUCCESS;
}

// -----------------------------------------------------------------------------
int
iAx25FrameSetDst (xAx25Frame *p, const char *callsign, uint8_t ssid) {

  if (!p) {

    return prviError (AX25_OBJECT_NOT_FOUND);
  }
  iAx25NodeSetCallsign (p->dst, callsign);
  p->dst->ssid = ssid;
  return AX25_SUCCESS;
}

// -----------------------------------------------------------------------------
int
iAx25FrameAddRepeater (xAx25Frame *p, const char *callsign, uint8_t ssid) {

  if (!p) {

    return prviError (AX25_OBJECT_NOT_FOUND);
  }
  if (p->repeaters_len >= AX25_MAX_RPT) {

    return prviError (AX25_ILLEGAL_REPEATER);
  }
  uint8_t index = p->repeaters_len++;

  p->repeaters[index] = xAx25NodeNew();
  iAx25NodeSetCallsign (p->repeaters[index], callsign);
  p->repeaters[index]->ssid = ssid;
  return index;
}

// -----------------------------------------------------------------------------
int
iAx25FrameSetRepeaterFlag (xAx25Frame *p, uint8_t index, bool flag) {

  if (!p) {

    return prviError (AX25_OBJECT_NOT_FOUND);
  }
  if (index >= p->repeaters_len) {

    return prviError (AX25_ILLEGAL_REPEATER);
  }

  p->repeaters[index]->flag = flag;
  return AX25_SUCCESS;
}

// -----------------------------------------------------------------------------
int
iAx25FrameSetInfo (xAx25Frame *p, const void *info, size_t info_len) {

  if ( (!p) || (!info)) {

    return prviError (AX25_OBJECT_NOT_FOUND);
  }

  info_len =  MIN (info_len, AX25_FRAME_BUF_LEN);
  memcpy (p->info, info, info_len);
  p->info_len = info_len;
  p->info[info_len] = 0; // prevents core dump
  return AX25_SUCCESS;
}

// -----------------------------------------------------------------------------
static char *
copyStrNode (char * dst, const xAx25Node *n) {
  char *str;

  str = xAx25NodeToStr (n);
  if (str) {

    memcpy (dst, str, strlen (str));
    dst += strlen (str);
    free (str);
  }
  return dst;
}

// -----------------------------------------------------------------------------
char *
xAx25FrameToStr (const xAx25Frame *f) {
  char *str = NULL;

  if (f) {

    str = malloc (FRAME_STRLEN (f->repeaters_len, f->info_len));
    if (str) {
      char *p = str;

      // NOCALL>TLM100,TEST*:>Test 00001: SolarPi (~) APRS http://www.btssn.net
      p = copyStrNode (p, f->dst);
      *p++ = '>';
      p = copyStrNode (p, f->src);

      for (int i = 0; i < f->repeaters_len; i++) {

        *p++ = ',';
        p = copyStrNode (p, f->repeaters[i]);
      }
      *p++ = ':';
      // copy the payload
      memcpy (p, f->info, f->info_len);
      p +=  f->info_len;
      *p = 0;
    }
  }
  else {
    prviError (AX25_OBJECT_NOT_FOUND);
  }

  return str;
}

// -----------------------------------------------------------------------------
int
iAx25FrameFilePrint (const xAx25Frame *p, FILE *f) {

  if (p) {

    if (f) {
      char *str = xAx25FrameToStr (p);

      if (str) {

        fprintf (f, "%s", str);
        free (str);
        return AX25_SUCCESS;
      }
      return prviError (AX25_NOT_ENOUGH_MEMORY);
    }
    return prviError (AX25_FILE_NOT_FOUND);
  }
  return prviError (AX25_OBJECT_NOT_FOUND);
}

// -----------------------------------------------------------------------------
int
iAx25FramePrint (const xAx25Frame *p) {

  return iAx25FrameFilePrint (p, stdout);
}

//##############################################################################
//#                                                                            #
//#                             xAx25 Class                                    #
//#                                                                            #
//##############################################################################

/* private ================================================================== */

// -----------------------------------------------------------------------------
static int
prviSetError (xAx25 *p, int iError) {

  p->error = iError;
  return prviError (iError);
}

#if AX25_CFG_USE_STREAM == 0
// -----------------------------------------------------------------------------
static int
prvxOpenFile (int fd, int mode, int unused) {
  int flag;

  if ( (flag = fcntl (fd, F_GETFL)) != -1) {

    flag &= O_ACCMODE;
    if ( (flag != mode) && (flag != O_RDWR)) {

      fd = -1;
    }
  }
  return fd;
}

// -----------------------------------------------------------------------------
static int
prviGetc (int fd) {
  uint8_t c;

  if (read (fd, &c, 1) <= 0) {

    return EOF;
  }
  return (unsigned) c;
}

// -----------------------------------------------------------------------------
static int
prviPutc (uint8_t c, int fd) {

  return write (fd, &c, 1);
}

#else
// TODO:  Pour l'instant l'accès flux ne fonctionne pas aucun caractère n'est
//        envoyé...
// -----------------------------------------------------------------------------
static FILE *
prvxOpenFile (int fd, int mode, FILE *other) {
  int flag;
  FILE *f = NULL;

  if (other) {
    int fdo = fileno (other);

    if ( (flag = fcntl (fdo, F_GETFL)) != -1) {

      flag &= O_ACCMODE;
      if (fd == fdo) {


        if ( (flag == mode) || (flag == O_RDWR)) {

          f = other;
          PDEBUG ("f = other\n");
        }
      }
    }
  }

  if (!f) {

    if ( (flag = fcntl (fd, F_GETFL)) != -1) {

      flag &= O_ACCMODE;
      if ( (flag == mode) || (flag == O_RDWR)) {

        if (flag ==  O_RDWR) {

          f = fdopen (fd, "w+");
          PDEBUG ("f = fdopen (fd, \"w+\")\n");
        }
        else if (flag ==  O_WRONLY) {

          f = fdopen (fd, "w");
          PDEBUG ("f = fdopen (fd, \"w\")\n");
        }
        else {

          f = fdopen (fd, "r");
          PDEBUG ("f = fdopen (fd, \"r\")\n");
        }
      }
      else {

        PWARNING ("The access mode of this file descriptor [%04X] is incompatible (%04X/%04X).\n", fd, flag, mode);
      }
    }
    else {

      PWARNING ("[%04X] is not an open file descriptor.\n", fd);
    }
  }

  return f;
}

// -----------------------------------------------------------------------------
static int
prviGetc (FILE *f) {

  return fgetc (f);
}

// -----------------------------------------------------------------------------
static int
prviPutc (uint8_t c, FILE *f) {

  return fputc (c, f);
}

#endif

/* public  ================================================================== */

// -----------------------------------------------------------------------------
xAx25 *
xAx25New (void) {
  xAx25 *p;

  p = malloc (sizeof (xAx25));
  if (p) {
    iAx25Clear (p);
  }
  return p;
}

// -----------------------------------------------------------------------------
void
vAx25Delete (xAx25 *p) {

  free (p);
}

// -----------------------------------------------------------------------------
int iAx25Clear (xAx25 *p) {

  if (!p) {

    return prviError (AX25_OBJECT_NOT_FOUND);
  }
  memset (p, 0, sizeof (xAx25));
  p->crc_in  = CRC_CCITT_INIT_VAL;
  p->crc_out = CRC_CCITT_INIT_VAL;

  return AX25_SUCCESS;
}

// -----------------------------------------------------------------------------
int
iAx25Error (xAx25 *p) {

  if (p) {

    return p->error;
  }
  (void) prviError (AX25_OBJECT_NOT_FOUND);
  return INT_MIN;
}

// -----------------------------------------------------------------------------
int
iAx25SetFdin (xAx25 *p, int fd) {

  if (!p) {

    return prviSetError (p, AX25_OBJECT_NOT_FOUND);
  }
  p->fin = prvxOpenFile (fd, O_RDONLY, p->fout);

  return prviSetError (p, AX25_SUCCESS);
}

// -----------------------------------------------------------------------------
int
iAx25SetFdout (xAx25 *p, int fd) {

  if (!p) {

    return prviSetError (p, AX25_OBJECT_NOT_FOUND);
  }
  p->fout = prvxOpenFile (fd, O_WRONLY, p->fin);

  return prviSetError (p, AX25_SUCCESS);
}

/*
 * ====================== Start of bertos code =================================
 * Bertos is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 * As a special exception, you may use this file as part of a free software
 * library without restriction.  Specifically, if other files instantiate
 * templates or use macros or inline functions from this file, or you compile
 * this file and link it with other files to produce an executable, this
 * file does not by itself cause the resulting executable to be covered by
 * the GNU General Public License.  This exception does not however
 * invalidate any other reasons why the executable file might be covered by
 * the GNU General Public License.
 *
 * Copyright 2009 Develer S.r.l. (http://www.develer.com/)
 * @author Francesco Sacchi <batt@develer.com>
 * Modified by Pascal JEAN <pjean@btssn.net>
 */
/* constants ================================================================ */
/*
 * Minimum size of a AX25 frame.
 */
#define AX25_MIN_FRAME_LEN 18

/*
 * CRC computation on correct AX25 packets should
 * give this result (don't ask why).
 */
#define AX25_CRC_CORRECT  0xF0B8

/* private ================================================================== */
// -----------------------------------------------------------------------------
static void
prvvPutChar (xAx25 *p, uint8_t c) {

  if (c == HDLC_FLAG || c == HDLC_RESET || c == AX25_ESC) {

    prviPutc (AX25_ESC, p->fout);
  }
  p->crc_out = usCrcCcittUpdate (c, p->crc_out);
  prviPutc (c, p->fout);
}

// -----------------------------------------------------------------------------
static int
prviSendCall (xAx25 *p, const xAx25Node *n, bool last) {
  unsigned len;

  len = MIN (AX25_CALL_LEN, strlen (n->callsign));

  for (unsigned i = 0; i < len; i++) {

    uint8_t c = n->callsign[i];
    if (! (isalnum (c) || c == ' ')) {

      return prviSetError (p, AX25_ILLEGAL_CALLSIGN);
    }
    c = toupper (c);
    prvvPutChar (p, c << 1);
    PINFO ("%02X", c << 1);
  }

  /* Fill with spaces the rest of the CALL if it's shorter */
  if (len < AX25_CALL_LEN) {
    uint8_t space = ' ' << 1;

    for (unsigned i = 0; i < (AX25_CALL_LEN - len); i++) {

      prvvPutChar (p, space);
      PINFO ("%02X", space);
    }
  }

  /* Bits6:5 should be set to 1 for all SSIDs (0x60) */
  /* The bit0 of last call SSID should be set to 1 */
  uint8_t ssid = 0x60 | (n->flag ? 0x80 : 0) | (n->ssid << 1) | (last ? 0x01 : 0);
  prvvPutChar (p, ssid);
  PINFO ("-%02X ", ssid);
  return prviSetError (p, AX25_SUCCESS);
}

// -----------------------------------------------------------------------------
uint8_t *
prvxExtractCallsign (uint8_t * buf, char * callsign) {

  for (uint8_t i = 0; i < AX25_CALL_LEN; i++) {

    char c = ( (*buf) >> 1);
    callsign[i] = (c == ' ') ? '\x0' : c;
    buf++;
  }
  return buf;
}

// -----------------------------------------------------------------------------
static int
prviDecode (xAx25 *p, xAx25Frame *f) {
  char cs[AX25_CALL_LEN];
  uint8_t *buf;

  buf = p->buf;
  iAx25FrameClear (f);

  buf = prvxExtractCallsign (buf, cs);
  PDEBUG ("Dst: %c%c%c%c%c%c\n", cs[0], cs[1], cs[2], cs[3], cs[4], cs[5]);
  iAx25FrameSetDst (f, cs, (*buf++ >> 1) & 0x0F);

  buf = prvxExtractCallsign (buf, cs);
  PDEBUG ("Src: %c%c%c%c%c%c\n", cs[0], cs[1], cs[2], cs[3], cs[4], cs[5]);
  iAx25FrameSetSrc (f, cs, (*buf >> 1) & 0x0F); // no ++ to test if there are repeaters

  PINFO ("SRC[%.6s-%d], DST[%.6s-%d]\n",  f->src->callsign,
         f->src->ssid,
         f->dst->callsign,
         f->dst->ssid);


  /* Repeater addresses */
  for (uint8_t index = 0; ! (*buf++ & 0x01) && (index < AX25_MAX_RPT); index++) {

    buf = prvxExtractCallsign (buf, cs);
    iAx25FrameAddRepeater (f, cs, (*buf >> 1) & 0x0F); // no ++ to test if the repeater was used

    // no ++ to test whether there are still repeaters
    iAx25FrameSetRepeaterFlag (f, index, (*buf & 0x80) ? true : false);

    PINFO ("RPT%d[%.6s-%d]%c\n",  index,
           f->repeaters[index]->callsign,
           f->repeaters[index]->ssid,
           (f->repeaters[index]->flag ? '*' : ' '));
  }

  f->ctrl = *buf++;
  if (f->ctrl != AX25_CTRL_UI) {

    PWARNING ("Only UI frames are handled, got [%02X]\n", f->ctrl);
    return AX25_INVALID_FRAME;
  }

  f->pid = *buf++;
  if (f->pid != AX25_PID_NOLAYER3) {

    PWARNING ("Only frames without layer3 protocol are handled, got [%02X]\n", f->pid);
    return AX25_INVALID_FRAME;
  }

  f->info_len = p->len - 2 - (buf - p->buf);
  memcpy (f->info, buf, f->info_len);
  PINFO ("DATA: %.*s\n", (int) f->info_len, f->info);

  // Ready for the next frame
  p->sync = false;
  p->crc_in = CRC_CCITT_INIT_VAL;
  p->len = 0;
  p->escape = false;
  p->frm_recv = false;

  return AX25_SUCCESS;
}

/* public  ================================================================== */

// -----------------------------------------------------------------------------
bool
bAx25Poll (xAx25 *p) {
  int c;

  if (!p->fin) {

    return false;
  }

  while ( (p->frm_recv == false) && ( (c = prviGetc (p->fin)) != EOF)) {

    if (!p->escape && c == HDLC_FLAG) {

      if (p->len >= AX25_MIN_FRAME_LEN) {

        if (p->crc_in == AX25_CRC_CORRECT) {

          PINFO ("Frame found!\n");
          p->frm_recv = true;
          break;  // TODO: sortie à éclaircir !!!
        }
        else {

          PINFO ("CRC error, computed [%04X]\n", p->crc_in);
        }
      }
      p->sync = true;
      p->crc_in = CRC_CCITT_INIT_VAL;
      p->len = 0;
      continue;
    }


    if (!p->escape && c == HDLC_RESET) {

      PINFO ("HDLC reset\n");
      p->sync = false;
      continue;
    }

    if (!p->escape && c == AX25_ESC) {

      p->escape = true;
      continue;
    }

    if (p->sync) {

      if (p->len < AX25_FRAME_BUF_LEN) {

        p->buf[p->len++] = c;
        p->crc_in = usCrcCcittUpdate (c, p->crc_in);
      }
      else {

        PINFO ("Buffer overrun");
        p->sync = false;
      }
    }
    p->escape = false;
  }

#if AX25_CFG_USE_STREAM
  if (ferror (p->fin)) {

    PWARNING ("File error [%04x]\n", ferror (p->fin));
    clearerr (p->fin);
  }
#endif

  return p->frm_recv;
}

// -----------------------------------------------------------------------------
int
iAx25Send (xAx25 *p, const xAx25Frame *f) {
  int iError;
  const uint8_t *buf = (const uint8_t *) f->info;
  size_t len;

  if (!p) {

    return prviError (AX25_OBJECT_NOT_FOUND);
  }
  if (!p->fout) {

    return AX25_FILE_NOT_FOUND;
  }
  if (!f) {

    return prviSetError (p, AX25_INVALID_FRAME);
  }

  p->crc_out = CRC_CCITT_INIT_VAL;
  prviPutc (HDLC_FLAG, p->fout);
  PINFO (">%02X ", HDLC_FLAG);

  /* Send path */
  iError = prviSendCall (p, f->dst, false);
  if (iError) {
    return iError;
  }
  iError = prviSendCall (p, f->src, (f->repeaters_len ? false : true));
  if (iError) {
    return iError;
  }

  for (uint8_t i = 0; i < f->repeaters_len; i++) {

    iError = prviSendCall (p, f->repeaters[i], (i == f->repeaters_len - 1));
    if (iError) {
      return iError;
    }
  }

  prvvPutChar (p, f->ctrl);
  PINFO ("%02X ", f->ctrl);

  prvvPutChar (p, f->pid);
  PINFO ("%02X ", f->pid);

  len = f->info_len;
  while (len--) {

    prvvPutChar (p, *buf);
    PINFO ("%c", *buf);
    buf++;
  }

  /*
   * According to AX25 protocol,
   * CRC is sent in reverse order!
   */
  uint8_t crcl = (p->crc_out & 0xff) ^ 0xff;
  uint8_t crch = (p->crc_out >> 8) ^ 0xff;
  prvvPutChar (p, crcl);
  PINFO (" %02X", crcl);
  prvvPutChar (p, crch);
  PINFO ("%02X ", crch);

  if (p->crc_out != AX25_CRC_CORRECT) {

    return prviSetError (p, AX25_CRC_ERROR);
  }

  prviPutc (HDLC_FLAG, p->fout);
  PINFO ("%02X\n", HDLC_FLAG);
  return prviSetError (p, AX25_SUCCESS);
}
/*
 * ======================= End of bertos code =================================
 */

// -----------------------------------------------------------------------------
int
iAx25Read (xAx25 *p, xAx25Frame *f) {

  if ( (!p) || (!f)) {

    return prviError (AX25_OBJECT_NOT_FOUND);
  }

  if (!p->frm_recv) {

    return AX25_NO_FRAME_RECEIVED;
  }

  p->error = AX25_SUCCESS;
  return prviDecode (p, f);
}

/* ========================================================================== */
