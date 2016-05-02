#!/usr/bin/env python
# -*- coding: utf-8 -*-
#
#  demo_aprs.py
#
#  Copyright 2014 epsilonRT, All rights reserved.
#  This software is governed by the CeCILL license <http://www.cecill.info>
#
from radio import *
import signal
import sys

# Parameters (global)
mytransmitperiod=3
mymsg='@>Test %05d: SolarPi APRS http:#www.btssn.net'
mycount=0
txenable=True

# -----------------------------------------------------------------------------
# Global variables
# The Tnc object
tnc=Tnc(256)
# The serial port
ser=Serial()

# -----------------------------------------------------------------------------
# Intercepts the CTRL + C signal and closes all properly
def signal_handler(signal, stack):
  print('\nserial port closed.\nHave a nice day !\n');
  ser.close()
  sys.exit(0)

# -----------------------------------------------------------------------------
# Send out message periodically...
def send_handler(signal, stack):
  global mycount

  mycount = mycount + 1
  payload = mymsg % mycount
  print('Send Frame: [%s]' % payload);
  tnc.write(payload)

# ------------------------------------------------------------------------------
def setup(dev,baud):
  # Opens the serial port
  ser.open(dev, baud)
  tnc.fdout = ser.fileno()
  tnc.fdin  = ser.fileno()

  # Installs the CTRL + C signal handler
  signal.signal(signal.SIGINT, signal_handler)

  # Setup periodic alarm
  if txenable == True:
    signal.signal(signal.SIGALRM, send_handler)
    signal.setitimer(signal.ITIMER_REAL, mytransmitperiod, mytransmitperiod)

# ------------------------------------------------------------------------------
def main():
  global txenable
  baud=38400
  dev=''
  index=1

  if len(sys.argv) < 2:
    print('Usage: %s /dev/ttyAMA0 [baud]' % sys.argv[0])
    sys.exit(-1)

  # Paramètre 1:
  if sys.argv[index] == '-n':
    txenable = False
  else:
    dev = sys.argv[index]
  index = index + 1

  # Paramètre 2:
  if len(sys.argv) > index:
    if txenable == True:
      baud = int(sys.argv[index])
    else:
      dev = sys.argv[index]
  index = index + 1

  # Paramètre 3:
  if len(sys.argv) > index:
    if txenable == True:
      if sys.argv[index] == '-n':
        txenable = False
    else:
      baud = int(sys.argv[index])

  setup(dev,baud)
  print('The tnc test is in progress on %s (baud=%d).' % (dev, baud))
  print('Transmitting=%s' % str(txenable))
  print('Press CTRL+C to quit\n')

  while True:
    # This function will look for new messages from the io channel.
    # It will call the xTncRead().
    if tnc.poll() == Tnc.APRS_PACKET:
      print('Recv Frame: [%s]' % tnc.read())

  return 0

# ------------------------------------------------------------------------------
if __name__ == '__main__':
  main()

# ------------------------------------------------------------------------------

