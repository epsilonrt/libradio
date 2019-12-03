// radio.i
%module radio
%{
  #include "../src/radio/ax25.hpp"
  #include "../src/radio/tnc.hpp"
  #include "../src/radio/serial.hpp"
%}



class Ax25Node {

public:
  Ax25Node(const char *str = "nocall");
  ~Ax25Node ();
  void clear();
  const char * toStr();

  void setCallsign (const char *str);
  const char * getCallsign() const;
  void setSsid (unsigned char ssid);
  unsigned char getSsid() const;
  void setFlag (bool flag);
  bool getFlag() const;

  %pythoncode %{
    __swig_getmethods__["callsign"] = getCallsign
    __swig_setmethods__["callsign"] = setCallsign
    if _newclass: callsign = property(getCallsign, setCallsign)
    __swig_getmethods__["ssid"] = getSsid
    __swig_setmethods__["ssid"] = setSsid
    if _newclass: ssid = property(getSsid, setSsid)
    __swig_getmethods__["flag"] = getFlag
    __swig_setmethods__["flag"] = setFlag
    if _newclass: flag = property(getFlag, setFlag)
  %}
  %extend {
    const char *__str__() {
      return $self->toStr();
    }
  };
};

class Ax25Frame {

public:
  static const unsigned char maxRepeater;

  enum {
    CTRL_UI = 0x03,
    PID_NOLAYER3 = 0xF0
  };

  Ax25Frame(const char *dst = "nocall", const char *src = "nocall");
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

  %pythoncode %{
    __swig_getmethods__["ctrl"] = getCtrl
    __swig_setmethods__["ctrl"] = setCtrl
    if _newclass: ctrl = property(getCtrl, setCtrl)

    __swig_getmethods__["pid"] = getPid
    __swig_setmethods__["pid"] = setPid
    if _newclass: pid = property(getPid, setPid)

    __swig_getmethods__["src"] = getSrc
    if _newclass: src = property(getSrc, None)

    __swig_getmethods__["dst"] = getDst
    if _newclass: dst = property(getDst, None)

    __swig_getmethods__["info"] = getInfo
    if _newclass: info = property(getInfo, None)

    __swig_getmethods__["infolen"] = getInfolen
    if _newclass: infolen = property(getInfolen, None)

    __swig_getmethods__["replen"] = getReplen
    if _newclass: replen = property(getReplen, None)
  %}
  %extend {
    const char *__str__() {
      return $self->toStr();
    }
  };
};

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
  int getFdout();
  void setFdin  (int fdi);
  int getFdin();

  bool poll();
  void send (const Ax25Frame &f);
  void read (Ax25Frame  &f);

  %pythoncode %{
    __swig_getmethods__["fdin"] = getFdin
    __swig_setmethods__["fdin"] = setFdin
    if _newclass: fdin = property(getFdin, setFdin)

    __swig_getmethods__["fdout"] = getFdout
    __swig_setmethods__["fdout"] = setFdout
    if _newclass: fdout = property(getFdout, setFdout)

    __swig_getmethods__["error"] = getError
    if _newclass: error = property(getError, None)
  %}
};

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
    IO_ERROR                 = -6
  };

  enum {

    NO_PACKET   = 0,
    APRS_PACKET = 1,
    SSDV_PACKET = 2,
    UNKN_PACKET = 3
  };

  Tnc(size_t iRxBufferSize = 80);
  ~Tnc ();
  int getError();

  void setFdout (int fdo);
  int getFdout() const;
  void setFdin  (int fdi);
  int getFdin() const;

  int poll();
  int getRxlen();
  int getPacketType(void) const;
  int getSsdvImageId() const;
  int getSsdvPacketId() const;
  const char * read(void) const;
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

  %pythoncode %{
    __swig_getmethods__["fdin"] = getFdin
    __swig_setmethods__["fdin"] = setFdin
    if _newclass: fdin = property(getFdin, setFdin)

    __swig_getmethods__["fdout"] = getFdout
    __swig_setmethods__["fdout"] = setFdout
    if _newclass: fdout = property(getFdout, setFdout)

    __swig_getmethods__["error"] = getError
    if _newclass: error = property(getError, None)

    __swig_getmethods__["rxlen"] = getRxlen
    if _newclass: rxlen = property(getRxlen, None)

    __swig_getmethods__["packet_type"] = getPacketType
    if _newclass: packet_type = property(getPacketType, None)

    __swig_getmethods__["ssdv_image_id"] = getSsdvImageId
    if _newclass: ssdv_image_id = property(getSsdvImageId, None)

    __swig_getmethods__["ssdv_packet_id"] = getSsdvPacketId
    if _newclass: ssdv_packet_id = property(getSsdvPacketId, None)
  %}
};

class Serial {

public:
  Serial();
  ~Serial();
  int open (const char *device, const int baud);
  void close();
  int fileno() const;
  void flush();
};
