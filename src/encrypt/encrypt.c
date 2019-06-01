
#include "encrypt.h"
#include "aes-256.h"
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <poll.h>
#include <sys/syscall.h>

char *defaultkey="b93nx7gm27x9rnw4kn29y7cbe5gnqpme";

void setup_connection(EncryptState *state, int local_port, int remote_port, char key[KEYBYTES]){
  // Set up the network state
    state->sockfd=socket(AF_INET,SOCK_DGRAM,0);

    bzero(&state->remote_addr,sizeof(state->remote_addr));
    state->remote_addr.sin_family = AF_INET;
    state->remote_addr.sin_addr.s_addr=inet_addr("127.0.0.1");
    state->remote_addr.sin_port=htons(remote_port);

    bzero(&state->local_addr, sizeof(state->local_addr));
    state->local_addr.sin_family = AF_INET;
    state->local_addr.sin_addr.s_addr=inet_addr("127.0.0.1");
    state->local_addr.sin_port = htons(local_port);
    bind(state->sockfd,(struct sockaddr *)&state->local_addr,sizeof(state->local_addr));

    //setup cipher state
    int sys=syscall(SYS_getrandom,state->iv,BLOCKBYTES,0);
    if(sys!=BLOCKBYTES){
      printf("Could not initialize connection.\n");
      exit(65);
    }
    memcpy(state->key,key,KEYBYTES);
    char *statekey=(char *)state->key;
    for(int i=0;i<KEYBYTES;i++){
      statekey[i]^=defaultkey[i];
    }
}

void close_connection(EncryptState *state){
  close(state->sockfd);
}

int get_response(EncryptState *state, InnerPacket *innerpacket){
  return send_packet(state,innerpacket) && recieve_packet(state,innerpacket);
}

int set_iv(EncryptState *state){
  char message[BLOCKBYTES];
  int sys=syscall(SYS_getrandom,message,BLOCKBYTES,0);
    if(sys!=BLOCKBYTES){
      return 0;
    }
  if(sendto(state->sockfd, message, BLOCKBYTES, 0,(struct sockaddr*) &state->remote_addr, sizeof(state->remote_addr))<0){
    return 0;
  }
  struct pollfd fds={};
  fds.fd=state->sockfd;
  fds.events=POLLIN;
  if(poll(&fds,1,10000)<=0){
    return 0;
  }
  char data[BLOCKBYTES*2+1];
  int datasize=recvfrom(state->sockfd, data, BLOCKBYTES*2+1, 0, NULL, NULL);
  if(datasize!=BLOCKBYTES*2){
    return 0;
  }
  aes_decrypt((int *)data,state->key);
  aes_decrypt((int *)(data+BLOCKBYTES),state->key);
  if(memcmp(data,message,BLOCKBYTES/2)
    ||memcmp(data+BLOCKBYTES,message+(BLOCKBYTES/2),BLOCKBYTES/2)){
    return 0;
  }
  memcpy(state->iv,data+(BLOCKBYTES/2),BLOCKBYTES/2);
  memcpy(state->iv+BLOCKWORDS/2,data+(3*BLOCKBYTES/2),BLOCKBYTES/2);
  return 1;
}

int send_packet(EncryptState *state, InnerPacket *innerpacket){
  
  char data[BLOCKCOUNT*BLOCKBYTES]={};
  OuterPacket *packet=(OuterPacket *)data;
  unsigned long checksum=0;
  packet->header=0x7D23EE84;
  packet->footer=0x9FA4CD68;
  memcpy(&packet->payload,innerpacket,sizeof(InnerPacket));
  for(int i=0;i<sizeof(OuterPacket)/4;i++){
    checksum^=((int *)data)[i];
  }
  packet->checksum=checksum;
  encrypt_blocks(state->iv,state->key,data,BLOCKCOUNT);
  
  if(sendto(state->sockfd, data, BLOCKCOUNT*BLOCKBYTES, 0,
	    (struct sockaddr*) &state->remote_addr, sizeof(state->remote_addr))<0){
    return 0;
  }
  return 1;
}

int recieve_packet(EncryptState *state, InnerPacket *innerpacket){
  struct pollfd fds={};
  fds.fd=state->sockfd;
  fds.events=POLLIN;
  if(poll(&fds,1,3000)<=0){
    return 0;
  }

  char data[BLOCKCOUNT*BLOCKBYTES]={};
  int datasize=recvfrom(state->sockfd, data, BLOCKCOUNT*BLOCKBYTES, 0, NULL, NULL);
  if(datasize!=BLOCKCOUNT*BLOCKBYTES)
    {
      if(datasize==BLOCKBYTES){
	int ivbuf[BLOCKWORDS];
        int sys=syscall(SYS_getrandom,ivbuf,BLOCKBYTES,0);
	if(sys==BLOCKBYTES){
	  memcpy(data+BLOCKBYTES,data+(BLOCKBYTES/2),BLOCKBYTES/2);
	  memcpy(data+(BLOCKBYTES/2),ivbuf,BLOCKBYTES/2);
	  memcpy(data+(3*BLOCKBYTES/2),ivbuf+(BLOCKWORDS/2),BLOCKBYTES/2);
	  aes_encrypt((int *)data,state->key);
	  aes_encrypt((int *)(data+BLOCKBYTES),state->key);
          datasize=sendto(state->sockfd, data, BLOCKBYTES*2, 0,(struct sockaddr*) &state->remote_addr, sizeof(state->remote_addr));
	  if(datasize==BLOCKBYTES*2){
	    memcpy(state->iv,ivbuf,BLOCKBYTES);
	  }
	}
      }
      return 0;
    }
  int ivbuf[BLOCKWORDS];
  memcpy(ivbuf,state->iv,BLOCKBYTES);
  decrypt_blocks(ivbuf,state->key,data,BLOCKCOUNT);
  OuterPacket *packet=(OuterPacket *)data;
  if(packet->header!=0x7D23EE84){
    return 0;
  }
  if(packet->footer!=0x9FA4CD68){
    return 0;
  }
  unsigned long checksum=0;
  for(int i=0;i<sizeof(OuterPacket)/4;i++){
    checksum^=((int *)data)[i];
  }
  if(checksum!=0){
    return 0;
  }
  memcpy(state->iv,ivbuf,BLOCKBYTES);
  memcpy(innerpacket,&packet->payload,sizeof(InnerPacket));
  return 1;
}

void encrypt_blocks(int iv[BLOCKWORDS], int key[KEYWORDS],char *blocks, int numBlocks){
  int *word=(int *)blocks;
  for(int i=0;i<numBlocks;i++){
    for(int j=0;j<BLOCKWORDS;j++){
      *word=(*word)^(iv[j]);
      word+=1;
    }
    aes_encrypt((int *)(blocks+i*BLOCKBYTES),key);
    memcpy(iv,blocks+i*BLOCKBYTES,BLOCKBYTES);
  }
}

void decrypt_blocks(int iv[BLOCKWORDS], int key[KEYWORDS],char *blocks, int numBlocks){
  int newiv[BLOCKWORDS];
  int *word=(int *)blocks;
  for(int i=0;i<numBlocks;i++){
    memcpy(newiv,blocks+i*BLOCKBYTES,BLOCKBYTES);
    aes_decrypt((int *)(blocks+i*BLOCKBYTES),key);
    for(int j=0;j<BLOCKWORDS;j++){
      *word=(*word)^(iv[j]);
      word+=1;
    }
    memcpy(iv,newiv,BLOCKBYTES);
  }
}


