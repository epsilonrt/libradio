/**
 * @file ax25.hpp
 * @brief Couche liaison du protocole AX.25 (C++).
 * 
 * Copyright © 2015 epsilonRT, All rights reserved.
 * This software is governed by the CeCILL license <http://www.cecill.info>
 */

#ifndef _SYSIO_AX25_HPP_
#define _SYSIO_AX25_HPP_

#include <string>

struct xAx25Node;
struct xAx25Frame;
struct xAx25;

/**
 *  @addtogroup radio_ax25
 *  @{
 *  @defgroup radio_ax25_cpp Interface C++
 *  @{
 */

/**
 * AX25 Node Class
 */
class Ax25Node {

public:
  Ax25Node(const char *callsign = "nocall");
  Ax25Node(struct xAx25Node *p);
  ~Ax25Node ();
  void clear();

  void setCallsign (const char *str);
  const char * getCallsign() const;

  void setSsid (unsigned char ssid);
  unsigned char getSsid() const;

  void setFlag (bool flag);
  bool getFlag() const;

  const char * toStr();

private:
  struct xAx25Node *p;
  std::string _tostr;
};

//##############################################################################
//#                                                                            #
//#                          xAx25Frame Class                                  #
//#                                                                            #
//##############################################################################
class Ax25;
/**
 * AX25 Frame Class
 */
class Ax25Frame {

public:
  static const unsigned char maxRepeater;
  enum {
    CTRL_UI = 0x03,
    PID_NOLAYER3 = 0xF0
  };

  Ax25Frame(const char *dst = "nocall", const char *src = "nocall");
  Ax25Frame (struct xAx25Frame *p);
  ~Ax25Frame ();
  void clear();

  Ax25Node & getSrc();
  Ax25Node & getDst();
  Ax25Node & repeater (unsigned char index);
  unsigned char getReplen() const;
  void addRepeater (const char *str, unsigned char ssid = 0);
  void setRepeaterFlag (unsigned char index, bool flag);

  void setCtrl (unsigned char ctrl);
  unsigned char getCtrl() const;

  void setPid (unsigned char pid);
  unsigned char getPid() const;

  void setInfo (const char *msg, size_t len);
  const char * getInfo() const;
  size_t getInfolen() const;

  const char * toStr();

private:
  friend class Ax25;
  std::string _tostr;
  Ax25Node *_src;
  Ax25Node *_dst;
  Ax25Node **_rep;

protected:
  struct xAx25Frame *p;
};

//##############################################################################
//#                                                                            #
//#                             xAx25 Class                                    #
//#                                                                            #
//##############################################################################
/**
 * AX25 Controller Class
 */
class Ax25 {

public:
  enum {
    CTRL_UI                  = 0x03,
    PID_NOLAYER3             = 0xF0,
    SUCCESS                  =  0,
    FILE_ACCESS_ERROR        = -1,
    ILLEGAL_CALLSIGN         = -2,
    INVALID_FRAME            = -3,
    CRC_ERROR                = -4,
    NOT_ENOUGH_MEMORY        = -5,
    NOT_ENOUGH_REPEATER      = -6,
    OBJECT_NOT_FOUND         = -7,
    FILE_NOT_FOUND           = -8,
    ILLEGAL_REPEATER         = -9,
    NO_FRAME_RECEIVED        = -10
  };

  Ax25();
  ~Ax25 ();
  void clear();
  int getError();

  void setFdout (int fdo);
  int getFdout() const;
  void setFdin  (int fdi);
  int getFdin() const;

  bool poll();
  void send (const Ax25Frame &f);
  void send (const Ax25Frame *f);
  void read (Ax25Frame  &f);
  void read (Ax25Frame  *f);

private:
  struct xAx25 *p;
};
/**
 *  @}
 * @}
 */

/* ========================================================================== */
#endif /* _SYSIO_AX25_HPP_ */
