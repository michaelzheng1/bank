#ifndef __ENCRYPT_H__
#define __ENCRYPT_H__

#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define BLOCKBYTES (128/8)
#define BLOCKWORDS (BLOCKBYTES/4)
#define KEYBYTES (256/8)
#define KEYWORDS (KEYBYTES/4)
#define BLOCKCOUNT ((sizeof(OuterPacket)-1)/BLOCKBYTES+1)

typedef enum _PacketType
{
  VERIFY_USERNAME,
  VERIFY_CARD,
  VERIFY_PIN,
  WITHDRAW,
  BALANCE_REQUEST,
  RESPONSE
} PacketType;

typedef struct _InnerPacket
{
  PacketType type;
  char username[256];
  int intPayload;
  char charPayload[128];
} InnerPacket;

typedef struct _OuterPacket
{
  int header;
  unsigned long checksum;
  InnerPacket payload;
  int footer;
} OuterPacket;
_Static_assert(sizeof(OuterPacket)%sizeof(unsigned long)==0,"OuterPacket cannot have an int checksum.");

typedef struct _EncryptState
{
  // Networking state
    int sockfd;
    struct sockaddr_in remote_addr;
    struct sockaddr_in local_addr;

  //cipher state
  int iv[BLOCKWORDS];
  int key[KEYWORDS];
} EncryptState;

int get_response(EncryptState *state, InnerPacket *innerpacket);
int set_iv(EncryptState *state);
int send_packet(EncryptState *state, InnerPacket *innerpacket);
int recieve_packet(EncryptState *state, InnerPacket *innerpacket);

void setup_connection(EncryptState *state, int local_port, int remote_port, char key[KEYBYTES]);
void close_connection(EncryptState *state);

void encrypt_blocks(int iv[BLOCKWORDS], int key[KEYWORDS],char *blocks, int numBlocks);
void decrypt_blocks(int iv[BLOCKWORDS], int key[KEYWORDS],char *blocks, int numBlocks);

#endif
