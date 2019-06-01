#ifndef COMMANDS_H
#define COMMANDS_H

#include <regex.h>
#include "formats.h"

#define RE_MATCH_MAX 5
#define MAXDIGITS 10
#define MAXDIGITS_SCANF_STR "%10d"

#define CMD_BUFFER_MAX 1000


typedef enum _commands_enum {
#ifndef _CP_ATM_MODE_
  CREATEUSER, BALANCE, DEPOSIT,
  BALANCE_INV, BALANCE_NOPARAMS,
  EMPTY, SPACES,
  CREATEUSER_INV, CREATEUSER_NOPARAMS,
  DEPOSIT_INV, DEPOSIT_NOPARAMS,
  INVALID
#else
  BEGIN_SESSION,
  ATM_BALANCE,
  PIN,
  END_SESSION,
  BALANCE_NOPARAMS,
  EMPTY, SPACES,
  ATM_WITHDRAW,
  BEGIN_SESSION_INV,
  BEGIN_SESSION_NOPARAMS,
  WITHDRAW_INV,
  WITHDRAW_NOPARAMS,
  INVALID
#endif
} CommandsEnum;


typedef struct _command_proc_pattern {
  char *re_string_form;
  regex_t *re_compiled_form;
  int number_of_subexpressions_to_capture;
} CmdProc_Pattern;


typedef struct _commands_layer {
  CmdProc_Pattern *patterns;
  regmatch_t mym[ RE_MATCH_MAX ];
  CommandsEnum t;
} CommandsLayer;


CommandsLayer *cp_create();
CommandsEnum cp_match_command( CommandsLayer *cl, char *command_line );
void copy_userinfo_from_commandline( User *user, CommandsLayer *cl, char *command_line );
void free_commands( CommandsLayer *cl );

#endif
