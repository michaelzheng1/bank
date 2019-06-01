/* 
 * The main program for the ATM.
 *
 */
#include "atm.h"
#include <stdio.h>
#include <stdlib.h>

static const char prompt[] = "ATM: ";
static const char pin_prompt[] = "PIN? ";
static const char loggedin_prompt[] = "ATM (%s): ";

int main(int argc, char **argv)
{
    char user_input[ CMD_BUFFER_MAX ];
    if(argc!=2){
      printf("Error opening ATM initialization file\n");
      return 64;
    }

    ATM *atm = atm_create(argv[1]);

    printf("%s", prompt);
    fflush(stdout);

    while (fgets(user_input, CMD_BUFFER_MAX, stdin) != NULL)
    {
        switch ( atm->state ) {
          case LOGGEDOUT:
            atm_process_command(atm, user_input);
            if ( atm->state == LOGGINGIN_CARD) {
              atm_process_card(atm);
            }
            print_atm_prompt_depending_if_state_changed_to
              ( atm->state, LOGGINGIN,
                "%s", pin_prompt, "%s", prompt );
                                       break;

          case LOGGINGIN_CARD:
                                       break;

          case LOGGINGIN:
            atm_process_pin(atm, user_input);
            print_atm_prompt_depending_if_state_changed_to
              ( atm->state, LOGGEDIN,
                loggedin_prompt, atm->login.username, "%s", prompt );
                                       break;

          case LOGGEDIN:
            atm_process_command(atm, user_input);
            print_atm_prompt_depending_if_state_changed_to
              ( atm->state, LOGGEDOUT,
                "%s", prompt, loggedin_prompt, atm->login.username );
                                       break;
        }
    }

  atm_free( atm );

	return EXIT_SUCCESS;
}
