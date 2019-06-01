#!/usr/bin/python3

import random as r
import prj414 as prj
import sys
import simulator as s

try:
  import pexpect
except ImportError:
  sys.stderr.write("Please install pexpect by running the ff.\n")
  sys.stderr.write("  sudo apt-get install python-pexpect python3-pexpect\n")
  exit(1)

try:
  basename = sys.argv[1]
except IndexError:
    sys.stderr.write("Please run as follows:\n")
    sys.stderr.write("  tests/tests.py <base name>\n")
    sys.stderr.write("Eg, if the file is named key.bank and key.atm\n")
    sys.stderr.write("  tests/tests.py key\n")
    sys.stderr.write("To generate those files do bin/init <base name>\n")
    exit(1)

router = pexpect.spawn( 'bin/router' )
bank = pexpect.spawn( 'bin/bank', [ '{}.bank'.format(basename) ] )
atm = pexpect.spawn( 'bin/atm', [ '{}.atm'.format(basename) ] )

bank.expect( 'BANK: ' )
atm.expect( 'ATM: ' )

sim_bank = s.Bank()
sim_atm = s.ATM( sim_bank )

def test_basic_create_balance_atm_login( bank=bank, atm=atm, sim_bank=sim_bank, sim_atm=sim_atm ):
  for name in [ prj.random_name( length = r.randint(3,5) ) for x in range( 5 ) ]:
    pin = prj.random_pin()

    bank.sendline( 'create-user ' + name + ' ' + pin + ' 0' )
    bank.expect( sim_bank.create_user( name, pin, 0 ) )
    bank.expect( '' )
    bank.expect( 'BANK: ' )

    bank.sendline( 'balance ' + name )
    bank.expect( sim_bank.balance( name ) )
    bank.expect( '' )
    bank.expect( 'BANK: ' )

    bank.sendline( 'deposit ' + name + ' 100' )
    bank.expect( sim_bank.deposit( name, 100 ) )
    bank.expect( '' )
    bank.expect( 'BANK: ' )

    atm.sendline( 'begin-session ' + name )
    atm.expect( 'PIN\? ' )
    atm.sendline( pin )
    atm.expect( sim_atm.begin_session( name, pin ) )
    atm.expect( '' )
    atm.expect( 'ATM \({}\): '.format(name) )

    atm.sendline( 'withdraw 101' )
    atm.expect( sim_atm.withdraw( 101 ) )
    atm.expect( '' )
    atm.expect( 'ATM \({}\): '.format(name) )

    atm.sendline( 'withdraw 100' )
    atm.expect( sim_atm.withdraw( 100 ) )
    atm.expect( '' )
    atm.expect( 'ATM \({}\): '.format(name) )

    bank.sendline( 'balance ' + name )
    bank.expect( sim_bank.balance( name ) )
    bank.expect( '' )
    bank.expect( 'BANK: ' )

    atm.sendline( 'balance' )
    atm.expect( sim_atm.balance() )
    atm.expect( '' )
    atm.expect( 'ATM \({}\): '.format(name) )

    atm.sendline( 'end-session' )
    atm.expect( sim_atm.end_session() )
    atm.expect( '' )
    atm.expect( 'ATM: ' )

test_basic_create_balance_atm_login()

atm.sendeof()
bank.sendeof()
router.sendintr()
