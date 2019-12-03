#!/usr/bin/env python
# -*- coding: utf-8 -*-
#
#  demo_ssdv.py
#
#  Copyright 2014 Pascal JEAN aka epsilonrt <pjean@btssn.net>
#
#  This program is free software; you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation; either version 2 of the License, or
#  (at your option) any later version.
#
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with this program; if not, write to the Free Software
#  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
#  MA 02110-1301, USA.
#
#
from radio import *
import signal
import sys
import os

# Parameters (global)
mytransmitperiod=1
mymsg='@>Test %05d: SolarPi APRS http://www.btssn.net'
txfilename="test"
rxfilename="test%03d.sdv"
callsign="solrpi"
txenable=True

# -----------------------------------------------------------------------------
# Global variables
# The Tnc object
tnc=Tnc(256)
# ssdv buffers
rxbuffer=tnc.malloc(256)
txbuffer=tnc.malloc(256)

packet_id=0
tximage_id=0
rximage_id=0

# The serial port
ser=Serial()

# -----------------------------------------------------------------------------
def new_ssdv_file():
  global callsign, tximage_id, txfilename

  os.system('ssdv -e -c %s -i %d %s.jpg tx%03d.sdv' % (callsign, tximage_id, txfilename, tximage_id))
  f=tnc.fopen('tx%03d.sdv' % tximage_id, 'r')
  tximage_id = tximage_id + 1
  return f

txfile=new_ssdv_file()
rxfile=tnc.fopen('rx%03d.sdv' % rximage_id, 'w')

# -----------------------------------------------------------------------------
# Intercepts the CTRL + C signal and closes all properly
def signal_handler(signal, stack):
  global rxbuffer, txbuffer, txfile

  print('\nserial port closed.\nHave a nice day !\n');
  ser.close()
  tnc.fclose(txfile)
  tnc.fclose(rxfile)
  tnc.free(rxbuffer)
  tnc.free(txbuffer)
  sys.exit(0)

# -----------------------------------------------------------------------------
# Send out message periodically...
def send_handler(signal, stack):
  global packet_id, txfile, packet_id, txbuffer, tximage_id

  if tnc.feof(txfile) != 0:
    tnc.fclose(txfile)
    txfile=new_ssdv_file()
    packet_id=0

  nbytes=tnc.fread(txbuffer, 1, 256, txfile)

  print('Tx Packet %d-%d' % (tximage_id - 1, packet_id));
  tnc.write(txbuffer, nbytes)
  packet_id = packet_id + 1

# ------------------------------------------------------------------------------
def setup(dev,baud):
  # Opens the serial port
  ser.open(dev,baud)
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
  global rxfile, rximage_id
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
    if tnc.poll() == Tnc.SSDV_PACKET:
      print('Rx Packet %d-%d' % (tnc.ssdv_image_id, tnc.ssdv_packet_id));

      if tnc.ssdv_image_id != rximage_id:
        tnc.fclose(rxfile)
        rximage_id = tnc.ssdv_image_id
        rxfile=tnc.fopen('rx%03d.sdv' % rximage_id, 'w')
      tnc.read(rxbuffer, tnc.rxlen)
      tnc.fwrite(rxbuffer, 1,  tnc.rxlen, rxfile)

  return 0

# ------------------------------------------------------------------------------
if __name__ == '__main__':
  main()

# ------------------------------------------------------------------------------

