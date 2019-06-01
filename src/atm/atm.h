/*
 * The ATM interfaces with the user.  User commands should be
 * handled by atm_process_command.
 *
 * The ATM can read .card files, but not .pin files.
 *
 */

#ifndef __ATM_H__
#define __ATM_H__

#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include "encrypt.h"
#include "commands.h"
#include "formats.h"

typedef enum _atm_state {
  LOGGEDOUT,
  LOGGINGIN_CARD,
  LOGGINGIN,
  LOGGEDIN
} ATMState;

typedef struct _ATM
{
  EncryptState encrypt_state;
  CommandsLayer *cp;
  ATMState state;
  User login;
} ATM;


ATM* atm_create(char *keyfile);
void atm_free(ATM *atm);
void atm_process_command(ATM *atm, char *command);
void atm_process_card(ATM *atm);
void atm_process_pin(ATM *atm, char *pin);
void print_atm_prompt_depending_if_state_changed_to( ATMState state, ATMState newstate,
  const char *new_string_format, const char *new_prompt, const char *same_string_format,
  const char *same_prompt );

int remote_verification(ATM *db, char *user, char pin[4], char *card);
int remote_withdraw(ATM *db, char *user, int amount);
int remote_balance(ATM *db, char *user);

#endif
