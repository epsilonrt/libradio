#!/usr/bin/env python
# -*- coding: utf-8 -*-
#
#  test_unit.py
#
#-----------------------------------------------------------------------------#
# Copyright © 2015 Pascal JEAN aka epsilonRT <pascal.jean--AT--btssn.net>     #
# All rights reserved.                                                        #
# This software is governed by the CeCILL license <http://www.cecill.info>    #
#-----------------------------------------------------------------------------#
from sysio import *
# import serial

device="/dev/ttyUSB0"
#device="/dev/ttyAMA0"
baudrate=2400

def NodeTest():
  print "Test Ax25Node object..."
  n=Ax25Node()
  n.callsign = "Test"
  assert n.callsign == "Test"
  n.ssid = 5
  assert n.ssid == 5
  n.flag = True
  assert n.flag == True
  print "Node: [%s]\nTest1 Success !" % n

  n.clear()
  assert n.callsign == ""
  assert n.ssid == 0
  assert n.flag == False
  print "Test2 Success !"

  n2=Ax25Node("StrStrStr")
  assert n2.callsign == "StrStr"
  assert n2.ssid == 0
  assert n2.flag == False
  print "Test3 Success !"

def FrameTest():
  print "Test Ax25Frame object..."

  f=Ax25Frame()
  f.src.callsign = "Source"
  assert f.src.callsign == "Source"
  f.src.ssid = 5
  assert f.src.ssid == 5
  f.src.flag = True
  assert f.src.flag == True
  f.src.flag = False
  assert f.src.flag == False
  f.src.flag = True
  assert f.src.flag == True
  print "Test1 Success !"

  f.dst.callsign = "Destin"
  assert f.dst.callsign == "Destin"
  f.dst.ssid = 12
  assert f.dst.ssid == 12
  f.dst.flag = True
  assert f.dst.flag == True
  f.dst.flag = False
  assert f.dst.flag == False
  print "Test2 Success !"

  s="012345678901234567890123456789"
  assert f.infolen == 0
  f.setInfo(s, len(s))
  assert f.infolen == len(s)
  assert f.info == s
  print "Test3 Success !"

  f.ctrl = 25
  assert f.ctrl == 25
  f.pid = 33
  assert f.pid == 33
  print "Test4 Success !"

  print "Frame: [%s]\nTest5 Success !" % f

  f.clear()
  assert f.infolen == 0
  assert f.info == ""
  assert f.replen == 0
  assert f.ctrl == Ax25.CTRL_UI
  assert f.pid == Ax25.PID_NOLAYER3
  assert f.src.callsign == ""
  assert f.src.ssid == 0
  assert f.src.flag == False
  assert f.dst.callsign == ""
  assert f.dst.ssid == 0
  assert f.dst.flag == False
  print "Test6 Success !"

  f.src.callsign = "Source"
  assert f.src.callsign == "Source"
  f.dst.callsign = "Destin"
  assert f.dst.callsign == "Destin"
  f.setInfo(s, len(s))
  assert f.infolen == len(s)
  assert f.info == s
  f.addRepeater("RepOk1")
  assert f.replen == 1
  assert f.repeater(0).callsign == "RepOk1"
  assert f.repeater(0).ssid == 0
  assert f.repeater(0).flag == False
  f.addRepeater("RepOk2")
  assert f.replen == 2
  assert f.repeater(1).callsign == "RepOk2"
  assert f.repeater(1).ssid == 0
  assert f.repeater(1).flag == False
  f.repeater(1).ssid = 2
  assert f.repeater(1).ssid == 2
  f.repeater(1).flag = True
  assert f.repeater(1).flag == True
  print "Frame: [%s]\nTest7 Success !" % f

def LinkTest():
  print "Test Ax25 object..."
  ax25=Ax25()
  assert ax25.error == Ax25.SUCCESS
  print "Test1 Success !"

  ser = Serial(device, baudrate)
  assert ser.open() == True
  print("Serial port infos :")
  print("  Port = %s" % ser.port)
  print("  Baudrate = %s" % ser.baudrate)
  print("  fd = %s" % ser.fileno)
  print "Test2 Success !"

  ax25.fdout =  ser.fileno
  assert ax25.fdout == ser.fileno
  ax25.fdin =  ser.fileno
  assert ax25.fdin == ser.fileno
  print "Test3 Success !"

  payload="012345678901234567890123456789012345678901234567890123456789"
  print "RXD line must be connected to TXD line for testing !"
  frx=Ax25Frame()
  ftx=Ax25Frame ("tlm100", "nocall")
  ftx.setInfo (payload, len(payload))
  print("Send Frame: [%s]" % ftx)
  ax25.send(ftx)
  recv=False
  while recv == False:
    if ax25.poll() == True :
      ax25.read(frx)
      print("Recv Frame: [%s]" % frx)
      recv=True
  assert frx.infolen == len(payload)
  assert frx.info == payload
  print "Test4 Success !"

  ser.close()

def main():
  NodeTest()
  FrameTest()
  LinkTest()
  return 0

if __name__ == '__main__':
  main()

