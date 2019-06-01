#ifndef __AES256_H__
#define __AES256_H__

void aes_encrypt(int block[4], int key[8]);
void aes_decrypt(int block[4], int key[8]);

void expand_key(int key[8],int keybuffer[15][4]);
void rotate_word(int word[1],int count);
void sub_word(int word[1]);
void sub_word_inverse(int word[1]);
void mix_columns(int block[4]);
void mix_columns_inverse(int block[4]);
void xor_block(int dest[4], int src[4]);

#endif
#ifdef BLOCKWORDS
_Static_assert(BLOCKWORDS==4,"Improper block size for aes-256.");
#endif
#ifdef KEYWORDS
_Static_assert(KEYWORDS==8,"Improper key size for aes-256.");
#endif
