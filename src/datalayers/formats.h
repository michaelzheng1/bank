#include <stdbool.h>

#ifndef __FORMATS_H
#define __FORMATS_H

typedef struct _User
{
  char username[256];
  char pin[4];
  int balance;
  bool balance_overflow;
  char card[128];
  unsigned long pintime;
  int pinattempts;
} User;

typedef enum _DatabaseResponse
{
  NETWORK_ERROR=-2,
  INVALID_REQUEST=-1,
  REQUEST_SUCCEEDED=1,
  REQUEST_FAILED=0
} DatabaseResponse;

#endif
