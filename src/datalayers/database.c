
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sys/syscall.h>
#include <unistd.h>
#include <time.h>
#include <limits.h>
#include "database.h"
#include "hash_table.h"


int db_add( Database *db, User *user )
{
  User *user_retrieved = hash_table_find( db->ht, user->username );

  if ( user_retrieved ) {
    return INVALID_REQUEST;
  }
  char card[128];
  int sys=syscall(SYS_getrandom,card,128,0);
  if(sys!=128){
    return REQUEST_FAILED;
  }
  User *user_to_add = malloc( sizeof( User ) );
  if(!user_to_add){
    return REQUEST_FAILED;
  }
  char cardname[300];
  snprintf(cardname,300,"./%s.card",user->username);
  FILE *fd=fopen(cardname,"w");
  if(fd==NULL){
    free(user_to_add);
    return REQUEST_FAILED;
  }
  int written=fwrite(card,1,128,fd);
  fclose(fd);
  if(written!=128){
    free(user_to_add);
    return REQUEST_FAILED;
  }
  memcpy( user_to_add->username, user->username, 256 );
  memcpy( user_to_add->pin, user->pin, 4 );
  memcpy(user_to_add->card,&card,128);
  user_to_add->balance = user->balance;
  user_to_add->pintime=0;
  user_to_add->pinattempts=0;
  hash_table_add( db->ht, user_to_add->username, user_to_add );
  return REQUEST_SUCCEEDED;
}


Database *db_create( void )
{
  Database *db = malloc( sizeof( Database ) );

  db->ht = hash_table_create( HASH_SIZE );

  return db;
}


int db_verification(Database *db, char *username, char pin[4], char card[128]){
  User *user=hash_table_find(db->ht,username);
  if(user==NULL){
    return REQUEST_FAILED;
  }
  if(card!=NULL&&memcmp(user->card,card,128)!=0){
    return REQUEST_FAILED;
  }
  if(pin!=NULL){
    if(user->pinattempts>=5){
      if(time(NULL)-user->pintime>=15*60){
	user->pinattempts=0;
      }else{
	return INVALID_REQUEST;
      }
    }
    if(memcmp(user->pin,pin,4)!=0){
      if(user->pinattempts==0){
        user->pintime=time(NULL);
      }
      user->pinattempts++;
      return REQUEST_FAILED;
    }
    user->pinattempts=0;
  }
  return REQUEST_SUCCEEDED; 
}


int db_deposit(Database *db, char *username, int amount){
  User *user=hash_table_find(db->ht,username);
  if(user==NULL){
    return INVALID_REQUEST;
  }
  if(amount>0&&user->balance>INT_MAX-amount)
    return REQUEST_FAILED;
  user->balance+=amount;
  return REQUEST_SUCCEEDED;
}


int db_withdraw(Database *db, char *username, int amount){
  User *user=hash_table_find(db->ht,username);
  if(user==NULL){
    return INVALID_REQUEST;
  }
  if(user->balance<amount){
    return REQUEST_FAILED;
  }
  user->balance-=amount;
  return REQUEST_SUCCEEDED;
}


int db_balance(Database *db, char *username){
  User *user=hash_table_find(db->ht,username);
  if(user==NULL){
    return INVALID_REQUEST;
  }
  return user->balance;
}


void db_destroy( Database *db )
{
  hash_table_free( db->ht );
  free( db );
}
