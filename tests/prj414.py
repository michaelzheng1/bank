#!/usr/bin/python3

BUFFER_LEN = 1000
NAME_MAX_LEN = 250
NAME_MIN_LEN = 1
MAX_AMOUNT = 2147483647

resp_inv_cmd = "Invalid command"
resp_create = "Created user {name}"
resp_create_usage = "Usage:  create-user <user-name> <pin> <balance>"
resp_balance = "\${amount}"
resp_balance_usage = "Usage:  balance <username>"
resp_no_such = "No such user"
resp_deposit_usage = "Usage:  deposit <username> <amt>"
resp_deposit = "\${amount} added to {name}'s account"

resp_begin_sess_usage = "Usage: begin-session <user-name>"
resp_begin_sess_already = "A user is already logged in"
resp_pin_auth = "Authorized"
resp_pin_not_auth = "Not authorized"
resp_atm_balance_usage = "Usage: balance"
resp_withdraw_usage = "Usage: withdraw <amt>"
resp_withdraw = "\${amount} dispensed"
resp_withdraw_insuff = "Insufficient funds"
resp_end_sess_out = "User logged out"
resp_not_loggedin = "No user logged in"


import string as s
import random

def random_char( alphabet=s.ascii_letters ):
  return alphabet[ random.randint( 0, len( alphabet ) - 1 ) ]

def random_name( length=NAME_MIN_LEN, english=True, alphabet=s.ascii_letters ):
  return ''.join( [ random_char( alphabet ) for x in range( length ) ] )

def random_names( qty=1, maxLen=NAME_MAX_LEN, english=True ):
  return [ random_name(length=random.randint(NAME_MIN_LEN, maxLen ) ) for x in range( qty ) ]

def random_pin():
  return random_name( length=4, alphabet=s.digits )
