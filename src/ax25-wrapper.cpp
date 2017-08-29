/**
 * @file src/ax25-wrapper.cpp
 * @brief Couche liaison du protocole AX.25 (Wrapper C++)
 * 
 * Copyright © 2015 epsilonRT, All rights reserved.
 * This software is governed by the CeCILL license <http://www.cecill.info>
 */
#include <radio/ax25.hpp>
#include <radio/ax25.h>
#include <stdlib.h>
#include <stdexcept>

//##############################################################################
//#                                                                            #
//#                         Ax25Node:: Class                                   #
//#                                                                            #
//##############################################################################

/* public  ================================================================== */

// -----------------------------------------------------------------------------
Ax25Node::Ax25Node (const char *cs) : p(xAx25NodeNew()) {

  setCallsign (cs);
}

// -----------------------------------------------------------------------------
Ax25Node::Ax25Node(struct xAx25Node *_p) : p(_p) {

}

// -----------------------------------------------------------------------------
Ax25Node::~Ax25Node () {

  vAx25NodeDelete (p);
}

// -----------------------------------------------------------------------------
void
Ax25Node::clear() {

  iAx25NodeClear (p);
}

// -----------------------------------------------------------------------------
void
Ax25Node::setCallsign (const char *str) {

  iAx25NodeSetCallsign (p, str);
}

// -----------------------------------------------------------------------------
const char *
Ax25Node::getCallsign() const {

  return p->callsign;
}

// -----------------------------------------------------------------------------
void
Ax25Node::setSsid (unsigned char ssid) {

  p->ssid = ssid;
}

// -----------------------------------------------------------------------------
unsigned char
Ax25Node::getSsid() const {

  return p->ssid;
}

// -----------------------------------------------------------------------------
void
Ax25Node::setFlag (bool flag) {

  p->flag = flag;
}

// -----------------------------------------------------------------------------
bool
Ax25Node::getFlag() const {

  return p->flag;
}

// -----------------------------------------------------------------------------
const char *
Ax25Node::toStr() {
  char *s;

  s = xAx25NodeToStr (p);
  _tostr = s;
  free (s);
  return _tostr.c_str();
}

//##############################################################################
//#                                                                            #
//#                            xAx25Frame Class                                #
//#                                                                            #
//##############################################################################

/* private ================================================================== */

/* public  ================================================================== */
const unsigned char Ax25Frame::maxRepeater = AX25_MAX_RPT;

// -----------------------------------------------------------------------------
Ax25Frame::Ax25Frame(const char *d, const char *s) :
  p(xAx25FrameNew()) {

  _src = new Ax25Node (p->src);
  _src->setCallsign (s);
  _dst = new Ax25Node (p->dst);
  _dst->setCallsign (d);
  _rep = new Ax25Node*[AX25_MAX_RPT];
}

// -----------------------------------------------------------------------------
Ax25Frame::Ax25Frame(struct xAx25Frame *_p) : p(_p) {

  _src = new Ax25Node (p->src);
  _dst = new Ax25Node (p->dst);
  _rep = new Ax25Node*[AX25_MAX_RPT];
  for (int i = 0; i < p->repeaters_len; i++) {

    _rep[i] = new Ax25Node (p->repeaters[i]);
  }
}

// -----------------------------------------------------------------------------
Ax25Frame::~Ax25Frame () {

  delete _src;
  delete _dst;
  for (unsigned char i = 0; i < p->repeaters_len; i++) {

    delete _rep[i];
  }
  delete _rep;
  delete p;
}

// -----------------------------------------------------------------------------
void
Ax25Frame::addRepeater(const char *str, unsigned char ssid) {

  int i = iAx25FrameAddRepeater (p, str, ssid);
  if (i >= 0) {

    _rep[i] = new Ax25Node (p->repeaters[i]);
  }
}

// -----------------------------------------------------------------------------
void
Ax25Frame::setRepeaterFlag(unsigned char index, bool flag) {

  iAx25FrameSetRepeaterFlag (p, index, flag);
}

// -----------------------------------------------------------------------------
void
Ax25Frame::clear() {

  for (unsigned char i = 0; i < p->repeaters_len; i++) {

    delete _rep[i];
  }
  iAx25FrameClear (p);
}

// -----------------------------------------------------------------------------
Ax25Node &
Ax25Frame::getSrc() {

  return *_src;
}

// -----------------------------------------------------------------------------
Ax25Node &
Ax25Frame::getDst() {

  return *_dst;
}

// -----------------------------------------------------------------------------
void
Ax25Frame::setCtrl (unsigned char ctrl) {

  p->ctrl = ctrl;
}

// -----------------------------------------------------------------------------
unsigned char
Ax25Frame::getCtrl() const{

  return p->ctrl;
}

// -----------------------------------------------------------------------------
void
Ax25Frame::setPid (unsigned char pid) {

  p->pid = pid;
}

// -----------------------------------------------------------------------------
unsigned char
Ax25Frame::getPid() const {

  return p->pid;
}

// -----------------------------------------------------------------------------
const char *
Ax25Frame::getInfo() const {

  return (const char *)p->info;
}
// -----------------------------------------------------------------------------
size_t
Ax25Frame::getInfolen() const {

  return p->info_len;
}

// -----------------------------------------------------------------------------
void
Ax25Frame::setInfo (const char *msg, size_t len) {

  iAx25FrameSetInfo (p, msg, len);
}

// -----------------------------------------------------------------------------
const char *
Ax25Frame::toStr() {
  char *s;

  s = xAx25FrameToStr (p);
  _tostr = s;
  free (s);
  return _tostr.c_str();
}

// -----------------------------------------------------------------------------
Ax25Node &
Ax25Frame::repeater(unsigned char i) {

  if (i >= p->repeaters_len) {

    throw std::out_of_range ("Ax25Frame::repeater");
  }
  return *_rep[i];
}

// -----------------------------------------------------------------------------
unsigned char
Ax25Frame::getReplen() const {

  return p->repeaters_len;
}

//##############################################################################
//#                                                                            #
//#                             xAx25 Class                                    #
//#                                                                            #
//##############################################################################

/* private ================================================================== */

/* public  ================================================================== */

// -----------------------------------------------------------------------------
Ax25::Ax25() : p(xAx25New()) {

  clear();
}

// -----------------------------------------------------------------------------
Ax25::~Ax25 () {

  vAx25Delete (p);
}

// -----------------------------------------------------------------------------
void
Ax25::clear () {

  iAx25Clear (p);
}

// -----------------------------------------------------------------------------
void
Ax25::setFdout (int fd) {

  iAx25SetFdout (p, fd);
}

// -----------------------------------------------------------------------------
int Ax25::getFdout() const {

  return p->fout;
}

// -----------------------------------------------------------------------------
int Ax25::getFdin() const {

  return p->fin;
}

// -----------------------------------------------------------------------------
void
Ax25::setFdin  (int fd) {

  iAx25SetFdin (p, fd);
}

// -----------------------------------------------------------------------------
bool
Ax25::poll() {

  return bAx25Poll (p);
}

// -----------------------------------------------------------------------------
void
Ax25::send (const Ax25Frame &f) {

  iAx25Send (p, f.p);
}

// -----------------------------------------------------------------------------
void
Ax25::read (Ax25Frame &f) {

  iAx25Read (p, f.p);
}

// -----------------------------------------------------------------------------
void
Ax25::send (const Ax25Frame *f) {

  iAx25Send (p, f->p);
}

// -----------------------------------------------------------------------------
void
Ax25::read (Ax25Frame *f) {

  iAx25Read (p, f->p);
}

// -----------------------------------------------------------------------------
int
Ax25::getError() {

  return iAx25Error (p);
}

/* ========================================================================== */
