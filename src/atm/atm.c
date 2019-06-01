#include "atm.h"
#include "ports.h"
#include "encrypt.h"
#include <string.h>
#include <stdlib.h>

ATM* atm_create(char *keyfilename)
{
    char keybuf[KEYBYTES+1];
    ATM *atm = (ATM*) malloc(sizeof(ATM));
    if(atm == NULL)
    {
        perror("Could not allocate ATM");
        exit(1);
    }

    FILE *keyfile=fopen(keyfilename,"r");
    if(keyfile<=0||fread(keybuf,1,KEYBYTES+1,keyfile)!=KEYBYTES){
      printf("Error opening ATM initialization file\n");
      exit(64);
    }
    fclose(keyfile);
    setup_connection(&(atm->encrypt_state),ATM_PORT,ROUTER_PORT,keybuf);
    if(!set_iv(&atm->encrypt_state)){
      perror("Error starting communication with server.");
      exit(65);
    }
    atm->cp = cp_create();
    atm->state = LOGGEDOUT;

    return atm;
}


void atm_free(ATM *atm)
{
    if(atm != NULL)
    {
      close_connection(&(atm->encrypt_state));
      free_commands( atm->cp);
      free(atm);
    }
}


void atm_process_card(ATM *atm)
{
  char cardname[300];
  snprintf(cardname,300,"./%s.card",atm->login.username);
  FILE *fd=fopen(cardname,"r");
  if(fd==NULL){
    atm->state = LOGGEDOUT;
    printf("Unable to access %s's card\n\n",atm->login.username);
    return;
  }
  int bytesread=fread(atm->login.card,1,128,fd);
  fclose(fd);
  if(bytesread!=128){
    atm->state = LOGGEDOUT;
    printf("Unable to access %s's card\n\n",atm->login.username);
    return;
  }
  atm->state = LOGGINGIN;
}


void atm_process_pin(ATM *atm, char *pin)
{
  if(cp_match_command(atm->cp,pin)!=PIN) {
    atm->state = LOGGEDOUT;
    printf( "Not authorized\n\n");
    return;
  }
  int result=remote_verification( atm, atm->login.username, pin, atm->login.card );
  if (result==REQUEST_SUCCEEDED) {
    atm->state = LOGGEDIN;
    printf( "Authorized\n\n");
    return;
  }
  atm->state = LOGGEDOUT;
  printf( "Not authorized\n\n");
}


void atm_process_command(ATM *atm, char *command)
{
  CommandsEnum cmd = cp_match_command( atm->cp, command );
  User user_cmd_fields;

  copy_userinfo_from_commandline( &user_cmd_fields, atm->cp, command );

  switch( cmd ) {
    int balance;
    int response;

    case BEGIN_SESSION:

      if ( atm->state == LOGGEDOUT ) {
        if ( remote_verification( atm, user_cmd_fields.username, NULL, NULL ) == REQUEST_SUCCEEDED ) {
          atm->state = LOGGINGIN_CARD;
          atm->login = user_cmd_fields;
        }
        else
          printf( "No such user\n" );
      }
      else
        printf( "A user is already logged in\n" );
                                       break;

    case ATM_BALANCE:

      if ( atm->state == LOGGEDIN ) {
        response = balance = remote_balance( atm, atm->login.username );
        if ( response != INVALID_REQUEST && response != NETWORK_ERROR )
          printf( "$%d\n", balance );
      }
      else
        printf( "No user logged in\n" );
                                       break;

    case ATM_WITHDRAW:

      if ( ! user_cmd_fields.balance_overflow ) {
        if ( atm->state == LOGGEDIN ) {
          response = remote_withdraw( atm, atm->login.username, user_cmd_fields.balance );
          if ( response == REQUEST_SUCCEEDED )
            printf( "$%d dispensed\n", user_cmd_fields.balance );
          else if ( response == REQUEST_FAILED )
            printf( "Insufficient funds\n" );
        }
        else
          printf( "No user logged in\n" );
      }
      else
        printf( "Invalid command\n" );
                                       break;

    case WITHDRAW_INV:
    case WITHDRAW_NOPARAMS:

      if ( atm->state == LOGGEDIN )
        printf( "Usage: withdraw <amt>\n" );
      else
        printf( "Invalid command\n" );
                                       break;

    case END_SESSION:

      if ( atm->state == LOGGEDIN ) {
        atm->state = LOGGEDOUT;
        atm->login.username[0] = '\0';
        printf( "User logged out\n" );
      }
      else
        printf( "No user logged in\n" );
                                       break;

    case EMPTY:
    case SPACES:
                                       break;

    case BEGIN_SESSION_INV:
    case BEGIN_SESSION_NOPARAMS:

      if ( atm->state == LOGGEDOUT )
        printf( "Usage:  begin-session <user-name>\n" );
      else
        printf( "Invalid command\n" );
                                       break;

    case BALANCE_NOPARAMS:

      if ( atm->state == LOGGEDIN )
        printf( "Usage: balance\n" );
                                       break;

    case PIN:
    case INVALID:

      printf( "Invalid command\n" );
                                       break;

    default:                           break;
  }

  if ( atm->state != LOGGINGIN_CARD && atm->state != LOGGINGIN ) printf( "\n" );
}


int remote_verification(ATM *atm, char *username, char pin[4], char *card){
  InnerPacket packet={};
  int out=REQUEST_SUCCEEDED;
  strncpy(packet.username,username,256);
  if(pin!=NULL){
    packet.type=VERIFY_PIN;
    memcpy(packet.charPayload,pin,4);
    if(!get_response(&(atm->encrypt_state),&packet)){
      return NETWORK_ERROR;
    }
    out=out && packet.intPayload==REQUEST_SUCCEEDED;
  }
  if(card!=NULL){
    packet.type=VERIFY_CARD;
    memcpy(packet.charPayload,card,128);
    if(!get_response(&(atm->encrypt_state),&packet)){
      return NETWORK_ERROR;
    }
    out=out && packet.intPayload==REQUEST_SUCCEEDED;
  }
  if(card==NULL && pin==NULL){
    packet.type=VERIFY_USERNAME;
    if(!get_response(&(atm->encrypt_state),&packet)){
      return NETWORK_ERROR;
    }
    out=out && packet.intPayload==REQUEST_SUCCEEDED;
  }
  return out?REQUEST_SUCCEEDED:REQUEST_FAILED;
}


int remote_withdraw(ATM *atm, char *username, int amount){
  InnerPacket packet={};
  packet.type=WITHDRAW;
  strncpy(packet.username,username,256);
  packet.intPayload=amount;
  if(!get_response(&(atm->encrypt_state),&packet)){
    return NETWORK_ERROR;
  }
  return packet.intPayload;
}


int remote_balance(ATM *atm, char *username){
  InnerPacket packet={};
  packet.type=BALANCE_REQUEST;
  strncpy(packet.username,username,256);
  if(!get_response(&(atm->encrypt_state),&packet)){
    return NETWORK_ERROR;
  }
  return packet.intPayload;
}


void print_atm_prompt_depending_if_state_changed_to( ATMState state, ATMState newstate,
  const char *new_string_format, const char *new_prompt, const char *same_string_format, const char *same_prompt )
{
  if ( state == newstate ) {
    printf(new_string_format, new_prompt);
  }
  else {
    printf(same_string_format, same_prompt);
  }

  fflush(stdout);
}
