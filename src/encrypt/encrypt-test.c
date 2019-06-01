#include "ports.h"
#include "encrypt.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>

int main(){
  printf("Freshness check means this test cannot work single threaded.");
  return 0;
  EncryptState alice_state={};
  EncryptState bob_state={};
  InnerPacket alice_packet={};
  InnerPacket bob_packet={};

  setup_connection(&alice_state,BANK_PORT,ATM_PORT,0);
  setup_connection(&bob_state,ATM_PORT,BANK_PORT,0);

  sendto(alice_state.sockfd, alice_state.iv, 16, 0,(struct sockaddr*) &alice_state.remote_addr, sizeof(alice_state.remote_addr));
  if(!set_iv(&bob_state)){
    printf("ERROR: Could not pair ivs.\n");
    exit(0);
  }
  recvfrom(alice_state.sockfd, NULL, 0, 0, NULL, NULL);

  for(int i=0;i<5;i++){
    int rnd=open("/dev/urandom", O_RDONLY);
    read(rnd, &alice_packet, sizeof(InnerPacket));
    close(rnd);

    int bool=1;
    bool=bool && send_packet(&alice_state,&alice_packet);
    bool=bool && recieve_packet(&bob_state,&bob_packet);
    if(!bool){
      printf("ERROR: Transmission reported failure.\n");
      exit(0);
    }

    int result=memcmp(&alice_packet,&bob_packet,sizeof(InnerPacket));
    if(result!=0){
      printf("ERROR: Corruption detected.\n");
      exit(0);
    }


    //reverse
    rnd=open("/dev/urandom", O_RDONLY);
    read(rnd, &bob_packet, sizeof(InnerPacket));
    close(rnd);

    bool=1;
    bool=bool && send_packet(&bob_state,&bob_packet);
    bool=bool && recieve_packet(&alice_state,&alice_packet);
    if(!bool){
      printf("ERROR: Transmission reported failure.\n");
      exit(0);
    }

    result=memcmp(&alice_packet,&bob_packet,sizeof(InnerPacket));
    if(result!=0){
      printf("ERROR: Corruption detected.\n");
      exit(0);
    }
  }
  printf("No corruption detected.\n");
}
