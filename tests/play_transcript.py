#!/usr/bin/python3
"""
To capture a transcript,

use a command similar to:

$ script tests/transcript_xxx.txt --command 'bin/bank key.bank'

This will record the interactive text into a file which you
can play back again using this tool.
"""
import sys
import re
import os

try:
  import pexpect
except ImportError:
  sys.stderr.write("Please install pexpect by running the ff.\n")
  sys.stderr.write("  sudo apt-get install python-pexpect python3-pexpect\n")
  exit(1)

try:
  base_name = sys.argv[1]
  transcript_name = sys.argv[2]
except IndexError:
  sys.stderr.write("Please run as follows:\n")
  sys.stderr.write("  {} <base name> tests/<transcript_file>\n\n".format( sys.argv[0] ) )
  sys.stderr.write("Eg, if the file is named key.bank and key.atm\n")
  sys.stderr.write("  {} key tests/<transcript_file>\n\n".format( sys.argv[0] ) )
  sys.stderr.write("To generate those files do bin/init <base name>\n")
  exit(1)

RESPONSE_TIMEOUT = 1 # in seconds

comment_line = re.compile( r'^Script.*|^#\s.*' )
directive_line = re.compile( r'### (.+) ###' )
bank_line = re.compile( r'^BANK: (.*)' )
atm_line = re.compile( r'^ATM(:| \([a-zA-Z]{1,250}\):) (.*)' )
pin_line = re.compile( r'^PIN\? (.*)' )
response_line = re.compile( r'(.*)' )

def run( directive ):
  if directive.startswith( 'remove_file ' ):
    filename = directive.replace( 'remove_file ', '' )
    os.remove( filename )

with open( transcript_name, 'r' ) as f:
  lines = f.readlines()

router = pexpect.spawn( 'bin/router' )
bank = pexpect.spawn( 'bin/bank', [ '{}.bank'.format(base_name) ] )
atm = pexpect.spawn( 'bin/atm', [ '{}.atm'.format(base_name) ] )

bank.expect( 'BANK: ', timeout=RESPONSE_TIMEOUT )
atm.expect( 'ATM: ', timeout=RESPONSE_TIMEOUT )

STATE = "START"

for linenum, transcript_line in enumerate( lines, start=1 ):

  if STATE == "START":
    if bank_line.match( transcript_line ):
      STATE = "ISSUE_BANK_COMMAND"
    elif atm_line.match( transcript_line ):
      STATE = "ISSUE_ATM_COMMAND"
    elif comment_line.match( transcript_line ):
      pass
    elif directive_line.match( transcript_line ):
      directive = directive_line.match( transcript_line ).group( 1 )
      run( directive )
  elif STATE == "EXPECT_BANK_RESPONSE":
    if bank_line.match( transcript_line ):
      STATE = "ISSUE_BANK_COMMAND"
    elif atm_line.match( transcript_line ):
      STATE = "ISSUE_ATM_COMMAND"
    elif comment_line.match( transcript_line ):
      pass
    elif directive_line.match( transcript_line ):
      directive = directive_line.match( transcript_line ).group( 1 )
      run( directive )
    else:
      expected_line = response_line.match( transcript_line ).group( 1 ).replace( '$', r'\$' )
      bank.expect( expected_line, timeout=RESPONSE_TIMEOUT )
  elif STATE == "EXPECT_ATM_RESPONSE":
    if bank_line.match( transcript_line ):
      STATE = "ISSUE_BANK_COMMAND"
    elif atm_line.match( transcript_line ):
      STATE = "ISSUE_ATM_COMMAND"
    elif comment_line.match( transcript_line ):
      pass
    elif pin_line.match( transcript_line ):
      STATE = "ISSUE_ATM_PIN"
    elif directive_line.match( transcript_line ):
      directive = directive_line.match( transcript_line ).group( 1 )
      run( directive )
    else:
      expected_line = response_line.match( transcript_line ).group( 1 ).replace( '$', r'\$' )
      atm.expect( expected_line, timeout=RESPONSE_TIMEOUT )

  if STATE == "ISSUE_BANK_COMMAND":
    print( '{:05} Bank {}'.format( linenum, bank_line.match( transcript_line ).group( 1 ) ) )
    bank.sendline( bank_line.match( transcript_line ).group( 1 ) )
    STATE = "EXPECT_BANK_RESPONSE"
  elif STATE == "ISSUE_ATM_COMMAND":
    print( '{:05}  ATM {}'.format( linenum, atm_line.match( transcript_line ).group( 2 ) ) )
    atm.sendline( atm_line.match( transcript_line ).group( 2 ) )
    STATE = "EXPECT_ATM_RESPONSE"
  elif STATE == "ISSUE_ATM_PIN":
    print( '{:05}  PIN {}'.format( linenum, pin_line.match( transcript_line ).group( 1 ) ) )
    atm.sendline( pin_line.match( transcript_line ).group( 1 ) )
    STATE = "EXPECT_ATM_RESPONSE"

atm.sendeof()
bank.sendeof()
router.sendintr()
