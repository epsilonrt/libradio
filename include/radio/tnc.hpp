/**
 * @file tnc.hpp
 * @brief Terminal Node Controller (C++)
 * 
 * Copyright © 2015 epsilonRT, All rights reserved.
 * This software is governed by the CeCILL license <http://www.cecill.info>
 */

#ifndef _SYSIO_TNC_HPP_
#define _SYSIO_TNC_HPP_

#include <string>
#include <stdio.h>
#include <sysio/defs.h>

struct xTnc;

/**
 *  @addtogroup radio_ax25_tnc
 *  @{
 *  @defgroup radio_ax25_tnc_cpp Interface C++
 *  @{
 */

/**
 * TNC Controller Class
 */
class Tnc {

public:
  enum {
    SOH = 1,
    STX = 2,
    ETX = 3,
    EOT = 4,
    TXT = 5,
    CRC = 6,
    SUCCESS                  =  0,
    CRC_ERROR                = -1,
    NOT_ENOUGH_MEMORY        = -2,
    OBJECT_NOT_FOUND         = -3,
    ILLEGAL_MSG              = -4,
    FILE_NOT_FOUND           = -5,
    IO_ERROR                 = -6,
  };

  typedef enum {

    NO_PACKET   = 0,
    APRS_PACKET = 1,
    SSDV_PACKET = 2,
    UNKN_PACKET = 3
  } ePacketType;

  Tnc(unsigned iRxBufferSize = 80);
  ~Tnc ();
  int getError();

  void setFdout (int fdo);
  int getFdout() const;
  void setFdin  (int fdi);
  int getFdin() const;

  int poll();
  int getRxlen();
  int getPacketType() const;
  int getSsdvImageId() const;
  int getSsdvPacketId() const;
  const char * read() const;
  int write (const char *buf);

  int write (const void  *buf, unsigned count);
  int read (void * buf, unsigned count);


  void * malloc (unsigned nbytes);
  void  free (void *);
  FILE *fopen (char * filename, char * mode);
  int fclose (FILE *f);
  unsigned fread(void *ptr, unsigned size, unsigned nobj, FILE * f);
  unsigned fwrite(void *ptr, unsigned size, unsigned nobj, FILE * f);
  int feof(FILE * f);
  int ferror(FILE * f);

private:
  struct xTnc *p;
  unsigned char *rx_buf;
  unsigned rx_buf_size;
  unsigned rx_len;
  ePacketType pkt_type;
  int ssdv_image_id;
  int ssdv_packet_id;
};
/**
 *  @}
 * @}
 */

/* ========================================================================== */
#endif /* _SYSIO_TNC_HPP_ */
