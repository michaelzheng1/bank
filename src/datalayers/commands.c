#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <limits.h>

#include "commands.h"

/* Reference:
   https://www.gnu.org/software/libc/manual/html_node/Regular-Expressions.html
*/

static CmdProc_Pattern cp_patterns[] = { 
/* Regular expression patterns, these have to be in the same order as
   the enums (in the header file) for index casting to work.
   
   Note that there are two sets, the second one is for the ATM.
   a compile option -D _CP_ATM_MODE is passed in to the compiler when
   compiling for the ATM; this way the bank code will not be present
   in the ATM binary and vice-versa.
*/
#ifndef _CP_ATM_MODE_
          { "^create-user ([a-zA-Z]{1,250}) ([0-9]{4}) ([0-9]+)\n?$",
                                    NULL, 3 },
          { "^balance ([a-zA-Z]{1,250})\n?$",
                                    NULL, 1 },
          { "^deposit ([a-zA-Z]{1,250}) ([0-9]+)\n?$",
                                    NULL, 2 },
          { "^balance .*\n?$",      NULL, 0 },
          { "^balance\n?$",         NULL, 0 },
          { "^\n?$",                NULL, 0 },
          { "^\\s*\n?$",            NULL, 0 },
          { "^create-user .*\n?$",  NULL, 0 },
          { "^create-user\n?$",     NULL, 0 },
          { "^deposit .*\n?$",      NULL, 0 },
          { "^deposit\n?$",         NULL, 0 },
#else
          { "^begin-session ([a-zA-Z]{1,250})\n?$",
                                    NULL, 1 },
          { "^balance\n?$",         NULL, 0 },
          { "^([0-9]{4})\n?$",      NULL, 1 },
          { "^end-session\n?$",     NULL, 0 },
          { "^balance .*\n?$",      NULL, 0 },
          { "^\n?$",                NULL, 0 },
          { "^\\s*\n?$",            NULL, 0 },
          { "^withdraw ([0-9]+)\n?$",
                                    NULL, 1 },
          { "^begin-session .*\n?$",NULL, 0 },
          { "^begin-session\n?$",   NULL, 0 },
          { "^withdraw .*\n?$",     NULL, 0 },
          { "^withdraw\n?$",        NULL, 0 },
#endif
};


static CmdProc_Pattern *compile_patterns( CmdProc_Pattern *patterns, int number_of_patterns )
{
  /* Allocate an array of regex_t */
  regex_t *compiled_pattern_ptr = malloc( sizeof( regex_t )
                                          * (size_t) number_of_patterns );

  /* Loop through the array of CmdProc_Patterns and compile the re_string_form to
     the re_compiled_form */
  for ( int i = 0; i < number_of_patterns; i++, compiled_pattern_ptr++ ) {
    
    if ( regcomp( patterns[ i ].re_compiled_form = compiled_pattern_ptr,
                  patterns[ i ].re_string_form, REG_EXTENDED ) ) {
      fprintf(stderr, "Could not compile regex\n");
      exit( EXIT_FAILURE );
    }

    /* A 0 here means there are no subexpressions, but if a positive number
       it needs to be incremented for reg_exec to work properly */
    patterns[ i ].number_of_subexpressions_to_capture =
      patterns[ i ].number_of_subexpressions_to_capture > 0 ?
      patterns[ i ].number_of_subexpressions_to_capture + 1 : 0;
  }
  return patterns;
}


CommandsLayer *cp_create()
{
    CommandsLayer *cl = malloc( sizeof( CommandsLayer ) );
    cl->patterns = compile_patterns( cp_patterns,
                                      sizeof( cp_patterns )
                                      / 
                                      sizeof( CmdProc_Pattern )  );
    cl->t = INVALID;

    return cl;
}


static void ensure_commandline_is_null_terminated( char *cmd_ptr)
{
  char *past_buffer = cmd_ptr + CMD_BUFFER_MAX;
  while ( *cmd_ptr != '\0' && cmd_ptr < past_buffer ) cmd_ptr++;
  if ( cmd_ptr >= past_buffer ) *(past_buffer-1) = '\0';
}

CommandsEnum cp_match_command( CommandsLayer *cl, char *command_line )
{
    CommandsEnum result = INVALID;

    ensure_commandline_is_null_terminated( command_line );

    /* Loop through each compiled regular expression and attempt to match it
       with the command line, in order.  The first to match (if any at all)
       will be set as the recognized command */
    for ( int i = 0; i < sizeof( cp_patterns ) / sizeof( CmdProc_Pattern ); i++  ) {
      if ( ! regexec( cl->patterns[ i ].re_compiled_form, command_line,
                      cl->patterns[ i ].number_of_subexpressions_to_capture, 
                      cl->patterns[ i ].number_of_subexpressions_to_capture > 0 ?
                                                                        cl->mym :
                                                                        NULL,
                                                                        0 ) ) {
        result = ( CommandsEnum ) i;
        break;
      }
    }

    return cl->t = result;
}


static void copy_subexpr_chars( char *copy_dest, char *command_line, regmatch_t *mym, int max, bool zeroit )
{
  for ( int i = (*mym).rm_so, j = 0; i < (*mym).rm_eo && j < max; i++, j++, copy_dest++ )
    *copy_dest = command_line[ i ];
  if ( zeroit ) *copy_dest = '\0';
}


static bool did_amount_overflow( CommandsLayer *cl, char *copy_src, char *end_ptr )
{
  while ( *copy_src == '0' && copy_src < end_ptr )
    copy_src++;
  if ( copy_src < end_ptr ) {
    if ( end_ptr - copy_src > MAXDIGITS )
      return true;
    else {
      unsigned long val;
      sscanf( copy_src, "%ld", &val );
      return val > INT_MAX ? true : false;
    }
  }
  else
    return false;
}

static char *skip_leading_zeroes( char *str ) { while ( *str == '0' ) str++; return str; }

void copy_userinfo_from_commandline( User *user, CommandsLayer *cl, char *command_line )
/* Note that there are two versions, the second one is for the ATM.
   a compile option -D _CP_ATM_MODE is passed in to the compiler when
   compiling for the ATM. */
{
#ifndef _CP_ATM_MODE_
  switch ( cl->t ) {
    char *copy_src;
    int max;

    case CREATEUSER:

      copy_subexpr_chars( user->username, command_line, &cl->mym[ 1 ], max=250, true );
      copy_subexpr_chars( user->pin, command_line, &cl->mym[ 2 ], max=4, false );

      copy_src = command_line + cl->mym[ 3 ].rm_so;
      if ( ! ( user->balance_overflow = did_amount_overflow( cl, copy_src, command_line + cl->mym[ 3 ].rm_eo ) ) ) {
        if ( ( copy_src = skip_leading_zeroes( copy_src ) ) < command_line + cl->mym[ 3 ].rm_eo )
          sscanf( copy_src, MAXDIGITS_SCANF_STR, &user->balance );
        else
          user->balance = 0;
      }
                                        break;
    case BALANCE:

      copy_subexpr_chars( user->username, command_line, &cl->mym[ 1 ], max=250, true );
                                        break;
    case DEPOSIT:

      copy_subexpr_chars( user->username, command_line, &cl->mym[ 1 ], max=250, true );

      copy_src = command_line + cl->mym[ 2 ].rm_so;
      if ( ! ( user->balance_overflow = did_amount_overflow( cl, copy_src, command_line + cl->mym[ 2 ].rm_eo ) ) ) {
        if ( ( copy_src = skip_leading_zeroes( copy_src ) ) < command_line + cl->mym[ 2 ].rm_eo )
          sscanf( copy_src, MAXDIGITS_SCANF_STR, &user->balance );
        else
          user->balance = 0;
      }
                                        break;
    default:
                                        break;
  }
#else
  switch ( cl->t ) {
    char *copy_src;
    int max;

    case BEGIN_SESSION:

      copy_subexpr_chars( user->username, command_line, &cl->mym[ 1 ], max=250, true );
                                        break;
    case ATM_WITHDRAW:

      copy_src = command_line + cl->mym[ 1 ].rm_so;
      if ( ! ( user->balance_overflow = did_amount_overflow( cl, copy_src, command_line + cl->mym[ 1 ].rm_eo ) ) ) {
        if ( ( copy_src = skip_leading_zeroes( copy_src ) ) < command_line + cl->mym[ 1 ].rm_eo )
          sscanf( copy_src, MAXDIGITS_SCANF_STR, &user->balance );
        else
          user->balance = 0;
      }
                                        break;
    default:
                                        break;
  }
#endif
}


void free_commands( CommandsLayer *cl )
{
     for ( int i = 0; i < sizeof( cp_patterns ) / sizeof( CmdProc_Pattern ); i++  ) {
       regfree( cl->patterns[ i ].re_compiled_form );
     }
     free( cl->patterns[ 0 ].re_compiled_form );
     free( cl );
}
