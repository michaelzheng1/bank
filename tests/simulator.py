#!/usr/bin/python3

import prj414 as prj
import re
from enum import Enum

class Bank:
  def __init__( self ):
    self._accounts = dict()
    self._keywords = dict()
    self._keywords['name'] = re.compile( r'[a-zA-Z]{{{min},{max}}}'.format( min=prj.NAME_MIN_LEN, max=prj.NAME_MAX_LEN ) )
    self._keywords['pin'] = re.compile( r'[0-9]{4}' )


  def create_user( self, name="", pin="", amount=0, card="" ):
    response = prj.resp_create_usage
    if self._keywords['name'].match( name ) and self._keywords['pin'].match( pin ) and amount >= 0 and amount <= prj.MAX_AMOUNT:
      self._accounts[ name ] = { 'pin':pin, 'amount':amount }
      response = prj.resp_create.format( name=name )

    return response


  def balance( self, name="" ):
    response = prj.resp_balance_usage
    if self._keywords['name'].match( name ):
      account = self._accounts.get( name, False )
      if account != False:
        response = prj.resp_balance.format( amount=account['amount'] )
      else:
        response = prj.resp_no_such

    return response
    

  def deposit( self, name="", amount=0 ):
    response = prj.resp_deposit_usage
    if self._keywords['name'].match( name ):
      account = self._accounts.get( name, False )
      if account != False:
        if amount + account['amount'] > prj.MAX_AMOUNT:
          response = prj.resp_too_rich
        else:
          account['amount'] += amount
          response = prj.resp_deposit.format( amount=amount, name=name )
      else:
        response = prj.resp_no_such

    return response


  def pin( self, name="" ):
    response = ""
    if self._keywords['name'].match( name ):
      account = self._accounts.get( name, False )
      if account != False:
        response = account['pin']

    return response


ATMState = Enum('ATMState', 'LOGGEDIN LOGGEDOUT')

class ATM():
  def __init__( self, bank ):
    self._bank = bank
    self._state = ATMState.LOGGEDOUT
    self._username = ""

  def begin_session( self, name="", pin="" ):
    response = prj.resp_begin_sess_usage
    if self._state == ATMState.LOGGEDOUT:
      if self._bank._keywords['name'].match( name ):
        account = self._bank._accounts.get( name, False )
        if account != False and account['pin']==pin:
          self._state = ATMState.LOGGEDIN
          self._username = name
          response = prj.resp_pin_auth
        else:
          response = prj.resp_pin_not_auth
      else:
        response = prj.resp_no_such
    elif self._state == ATMState.LOGGEDIN:
      response = prj.resp_begin_sess_already

    return response

  def balance( self ):
    response = prj.resp_atm_balance_usage
    if self._state == ATMState.LOGGEDOUT:
      response = prj.resp_not_loggedin
    else:
      response = self._bank.balance( self._username )

    return response

  def end_session( self ):
    response = prj.resp_inv_cmd
    if self._state == ATMState.LOGGEDOUT:
      response = prj.resp_not_loggedin
    elif self._state == ATMState.LOGGEDIN:
      self._state = ATMState.LOGGEDOUT
      self._username = ""
      response = prj.resp_end_sess_out

    return response


  def withdraw( self, amount=0 ):
    response = prj.resp_withdraw_usage
    if self._state == ATMState.LOGGEDOUT:
      response = prj.resp_not_loggedin
    elif self._state == ATMState.LOGGEDIN:
      account = self._bank._accounts.get( self._username, False )
      if account != False:
        if amount > account['amount']:
          response = prj.resp_withdraw_insuff
        else:
          account['amount'] -= amount
          response = prj.resp_withdraw.format( amount=amount )

    return response
