/* 
 * The main program for the Bank.
 *
 */

#include <sys/select.h>
#include <stdio.h>
#include <stdlib.h>
#include "bank.h"
#include "ports.h"
#include "encrypt.h"
#include "database.h"

static const char prompt[] = "BANK: ";

int main(int argc, char**argv)
{
   char sendline[ CMD_BUFFER_MAX ];
   if(argc!=2){
     printf("Error opening bank initialization file\n");
     return 64;
   }

   Bank *bank = bank_create(argv[1]);

   printf("%s", prompt);
   fflush(stdout);

   while(1)
   {
       fd_set fds;
       FD_ZERO(&fds);
       FD_SET(0, &fds);
       FD_SET(bank->encrypt_state.sockfd, &fds);
       select(bank->encrypt_state.sockfd+1, &fds, NULL, NULL, NULL);

       if(FD_ISSET(0, &fds))
       {
           if ( fgets(sendline, CMD_BUFFER_MAX, stdin) != NULL ) {
             bank_process_local_command(bank, sendline);
             printf("%s", prompt);
             fflush(stdout);
           }
           else
             break;
       }
       else if(FD_ISSET(bank->encrypt_state.sockfd, &fds))
       {
	 InnerPacket packet={};
           int success=recieve_packet(&(bank->encrypt_state),&packet);
	   if(!success){
             continue;
	   }
	   packet.username[255]=0;
           int out=-1;
           switch(packet.type){
             case VERIFY_USERNAME: out=db_verification(bank->db,packet.username,NULL,NULL);break;
             case VERIFY_CARD:     out=db_verification(bank->db,packet.username,NULL,packet.charPayload);break;
             case VERIFY_PIN:      out=db_verification(bank->db,packet.username,packet.charPayload,NULL);break;
             case WITHDRAW:        out=db_withdraw(bank->db,packet.username,packet.intPayload);break;
             case BALANCE_REQUEST: out=db_balance(bank->db,packet.username);break;
           default:break;
           }
           packet.type=RESPONSE;
           packet.intPayload=out;
           send_packet(&(bank->encrypt_state),&packet);
       }
   }

   bank_free( bank );

   return EXIT_SUCCESS;
}
