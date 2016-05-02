/**
 * @file sysio++/src/tnc.cpp
 * @brief Terminal Node Controller (Wrapper C++)
 *
 * Copyright © 2015 epsilonRT, All rights reserved.
 * This software is governed by the CeCILL license <http://www.cecill.info>
 */
#include <radio/tnc.hpp>
#include <radio/tnc.h>
#include <ssdv/ssdv.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdexcept>

//##############################################################################
//#                                                                            #
//#                             xTnc Class                                    #
//#                                                                            #
//##############################################################################

/* public  ================================================================== */

// -----------------------------------------------------------------------------
Tnc::Tnc (unsigned iRxBufferSize) : p (xTncNew (iRxBufferSize)), pkt_type (NO_PACKET) {

  rx_buf = new unsigned char[iRxBufferSize];
  rx_buf_size = iRxBufferSize;
}

// -----------------------------------------------------------------------------
Tnc::~Tnc () {

  delete rx_buf;
  vTncDelete (p);
}

// -----------------------------------------------------------------------------
void
Tnc::setFdout (int fd) {

  iTncSetFdout (p, fd);
}

// -----------------------------------------------------------------------------
int Tnc::getFdout() const {

  return p->fout;
}

// -----------------------------------------------------------------------------
int Tnc::getFdin() const {

  return p->fin;
}

// -----------------------------------------------------------------------------
void
Tnc::setFdin (int fd) {

  iTncSetFdin (p, fd);
}

// -----------------------------------------------------------------------------
int
Tnc::poll() {
  int state, rv = NO_PACKET;

  state = iTncPoll (p);
  if (state == TNC_EOT) {

    memcpy (rx_buf, p->rxbuf, p->len);
    rx_len = p->len;

    if (rx_len == SSDV_PKT_SIZE) {
      int iError;

      // Le paquet a la longueur d'un paquet SSDV
      if (ssdv_dec_is_packet (rx_buf, &iError) == 0) {
        ssdv_packet_info_t xPktInfo;

        // C'est un paquet SSDV
        ssdv_dec_header (&xPktInfo, rx_buf);
        ssdv_image_id = (unsigned int) xPktInfo.image_id;
        ssdv_packet_id = (unsigned int) xPktInfo.packet_id;
        rv = SSDV_PACKET;
        pkt_type = (ePacketType) rv;
      }
    }
    else {

      if ( (rx_buf[0] == '@') || (rx_buf[0] == '/')) {

        // C'est un paquet APRS
        if (rx_len < rx_buf_size) {

          rv = APRS_PACKET;
          pkt_type = (ePacketType) rv;
          rx_buf[rx_len] = 0;
          //printf("APRS Packet: %s\n", rx_buf);
        }
        else {

          fprintf (stderr, "APRS packet too long, maximum length = %d\n", rx_buf_size - 1);
        }
      }
    }
  }
  return rv;
}

// -----------------------------------------------------------------------------
int
Tnc::getPacketType() const {

  return (int) pkt_type;
}

// -----------------------------------------------------------------------------
int
Tnc::getSsdvImageId() const {

  return ssdv_image_id;
}

// -----------------------------------------------------------------------------
int
Tnc::getSsdvPacketId() const {

  return ssdv_packet_id;
}

// -----------------------------------------------------------------------------
int
Tnc::getRxlen() {

  return rx_len;
}

// -----------------------------------------------------------------------------
int
Tnc::write (const void *buf, unsigned count) {

  return iTncWrite (p, buf, count);
}

// -----------------------------------------------------------------------------
int
Tnc::write (const char *buf) {

  return iTncWrite (p, buf, strlen (buf));
}

// -----------------------------------------------------------------------------
int
Tnc::read (void *buf, unsigned count) {
  int s = MIN (count, rx_len);

  memcpy (buf, rx_buf, s);
  return s;
}

// -----------------------------------------------------------------------------
const char *
Tnc::read() const {

  return (const char *) rx_buf;
}

// -----------------------------------------------------------------------------
int
Tnc::getError() {

  return iTncError (p);
}

// -----------------------------------------------------------------------------
FILE *
Tnc::fopen (char *filename, char *mode) {
  FILE * f;

  f = ::fopen (filename, mode);
  if (f == NULL) {

    fprintf (stderr, "Error openning file %s: %s\n", filename, strerror (errno));
    return 0;
  }
  return f;
}
// -----------------------------------------------------------------------------
int
Tnc::fclose (FILE *f) {

  return ::fclose (f);
}

// -----------------------------------------------------------------------------
int
Tnc::feof (FILE * f) {

  return ::feof (f);
}

// -----------------------------------------------------------------------------
int
Tnc::ferror (FILE * f) {

  return ::ferror (f);
}

// -----------------------------------------------------------------------------
unsigned
Tnc::fread (void *ptr, unsigned size, unsigned nobj, FILE * f) {

  return ::fread (ptr, size, nobj, f);
}
// -----------------------------------------------------------------------------
unsigned
Tnc::fwrite (void *ptr, unsigned size, unsigned nobj, FILE * f) {

  return ::fwrite (ptr, size, nobj, f);
}

// -----------------------------------------------------------------------------
void *
Tnc::malloc (unsigned nbytes) {

  return::malloc (nbytes);
}

// -----------------------------------------------------------------------------
void
Tnc::free (void * ptr) {

  ::free (ptr);
}

/* ========================================================================== */
