#include "ports.h"
#include "bank.h"
#include <stdlib.h>


Bank* bank_create(char *keyfilename)
{
    char keybuf[KEYBYTES+1];
    Bank *bank = (Bank*) malloc(sizeof(Bank));
    if(bank == NULL)
    {
        perror("Could not allocate Bank");
        exit(1);
    }

    FILE *keyfile=fopen(keyfilename,"r");
    if(keyfile<=0||fread(keybuf,1,KEYBYTES+1,keyfile)!=KEYBYTES){
      printf("Error opening bank initialization file\n");
      exit(64);
    }
    fclose(keyfile);
    setup_connection(&(bank->encrypt_state),BANK_PORT,ROUTER_PORT,keybuf);

    bank->cp = cp_create();
    bank->db = db_create();

    return bank;
}


void bank_free(Bank *bank)
{
    if(bank != NULL)
    {
      close_connection(&(bank->encrypt_state));
      db_destroy( bank->db );
      free_commands( bank->cp );
      free(bank);
    }
}


void bank_process_local_command(Bank *bank, char *command)
{
    CommandsEnum cmd = cp_match_command( bank->cp, command );
    User user_cmd_fields;
    int balance;

    switch ( cmd ) {

      case CREATEUSER:

        copy_userinfo_from_commandline( &user_cmd_fields, bank->cp, command );

        if ( ! user_cmd_fields.balance_overflow ) {
          int response=db_add( bank->db, &user_cmd_fields );
          if (response==REQUEST_SUCCEEDED) {
            printf( "Created user %s\n", user_cmd_fields.username );
          }
          else if(response==INVALID_REQUEST){
            printf( "Error:  user %s already exists\n", user_cmd_fields.username );
          }else{
            printf("Error creating card file for user %s\n",user_cmd_fields.username);
          }
        }
        else {
          printf( "Usage:  create-user <user-name> <pin> <balance>\n" );
        }
                                       break;
      case BALANCE:

        copy_userinfo_from_commandline( &user_cmd_fields, bank->cp, command );

        int response = balance = db_balance( bank->db, user_cmd_fields.username );
        if ( response != INVALID_REQUEST ) {
          printf( "$%d\n", balance );
        }
        else
          printf( "No such user\n" );
                                       break;
      case DEPOSIT:

        copy_userinfo_from_commandline( &user_cmd_fields, bank->cp, command );

        if ( ! user_cmd_fields.balance_overflow ) {

          response = db_deposit( bank->db, user_cmd_fields.username, user_cmd_fields.balance );
          if ( response ==  REQUEST_SUCCEEDED )
            printf( "$%d added to %s's account\n", user_cmd_fields.balance, user_cmd_fields.username );
          else if ( response == REQUEST_FAILED )
            printf( "Too rich for this program\n" );
          else if ( response == INVALID_REQUEST )
            printf( "No such user\n" );
        }
        else {
          printf( "Usage:  deposit <user-name> <amt>\n" );
        }

                                       break;
      case CREATEUSER_INV:
      case CREATEUSER_NOPARAMS:
        printf( "Usage:  create-user <user-name> <pin> <balance>\n" );
                                       break;
      case BALANCE_INV:
      case BALANCE_NOPARAMS:
        printf( "Usage:  balance <user-name>\n" );
                                       break;
      case DEPOSIT_INV:
      case DEPOSIT_NOPARAMS:
        printf( "Usage:  deposit <user-name> <amt>\n" );
                                       break;
      case EMPTY:
      case SPACES:
                                       break;
      case INVALID:
        printf( "Invalid command\n" );
                                       break;
    }

    printf( "\n" );
}
