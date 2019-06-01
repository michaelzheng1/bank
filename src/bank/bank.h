/*
 * The Bank takes commands from stdin as well as from the ATM.  
 *
 * Commands from stdin be handled by bank_process_local_command.
 *
 * Remote commands from the ATM should be handled by
 * bank_process_remote_command.
 *
 * The Bank can read both .card files AND .pin files.
 *
 */

#ifndef __BANK_H__
#define __BANK_H__

#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include "encrypt.h"
#include "commands.h"
#include "database.h"
#include "hash_table.h"
#include "formats.h"

#define RE_CREATEUSER 0
#define RE_BALANCE 1
#define RE_DEPOSIT 2

typedef struct _Bank
{
    EncryptState encrypt_state;

    CommandsLayer *cp;
    Database *db;
} Bank;


Bank* bank_create(char *keyfile);
void bank_free(Bank *bank);
void bank_process_local_command(Bank *bank, char *command);
void bank_process_remote_command(Bank *bank, char *command, size_t len);

#endif

