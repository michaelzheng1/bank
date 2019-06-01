
#ifndef __DATABASE_H
#define __DATABASE_H

#include "hash_table.h"
#include "formats.h"

#define HASH_SIZE 26*2*100

typedef struct _Database
{
  HashTable *ht;
} Database;


Database *db_create( void );
int db_add( Database *db, User *user );
int db_verification(Database *db, char *user, char pin[4], char *card);
int db_deposit(Database *db, char *user, int amount);
int db_withdraw(Database *db, char *user, int amount);
int db_balance(Database *db, char *user);
void db_destroy( Database *db );

#endif

