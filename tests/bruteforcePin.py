#!/usr/bin/python3
import sys
import simulator as s
import os
try:
  import pexpect
except ImportError:
  sys.stderr.write("Please install pexpect by running the ff.\n")
  sys.stderr.write("  sudo apt-get install python-pexpect python3-pexpect\n")
  exit(1)


#cardfile = raw_input("Enter cardfile: ")
#cmd = './atm ' + cardfile
#cd ../bin && ./atm test.atm
#print cmd

basename = "test"
atm = pexpect.spawn( 'bin/atm', [ '{}.atm'.format(basename) ] )

atm.expect( 'ATM: ' )
print "ATM"

atm.sendline( 'begin-session Alice' )
atm.expect( 'PIN\? ' )
print "PIN?"

pin = 0000;
pin_str =  '{0:04d}'.format(pin)
print pin_str

atm.sendline( pin_str )
atm.expect( sim_atm.begin_session( 'Alice', '0000' ) )
print "Authroized"
