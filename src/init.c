#include <stdio.h>
#include <sys/syscall.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char **argv){
  if(argc!=2){
    printf("Usage: init <filename>\n");
    return 62;
  }
  int bufsize=strlen(argv[1])+10;
  char *bankname=malloc(bufsize);
  char *atmname=malloc(bufsize);
  if(!(bankname&&atmname)){
    printf("Error creating initialization files\n");
    free(bankname);
    free(atmname);
    return 64;
  }
  snprintf(bankname,bufsize,"./%s.bank",argv[1]);
  snprintf(atmname,bufsize,"./%s.atm",argv[1]);
  FILE *bankfile=fopen(bankname,"r");
  FILE *atmfile=fopen(atmname,"r");
  if(bankfile>0||atmfile>0){
    fclose(bankfile);
    fclose(atmfile);
    free(bankname);
    free(atmname);
    printf("Error: one of the files already exists\n");
    return 63;
  }
  bankfile=fopen(bankname,"w");
  atmfile=fopen(atmname,"w");
  free(bankname);
  free(atmname);
  if(bankfile<=0||atmfile<=0){
    fclose(bankfile);
    fclose(atmfile);
    printf("Error creating initialization files\n");
    return 64;
  }
  char key[32];
  int sys=syscall(SYS_getrandom,&key,32,0);
  if(sys!=32){
    fclose(bankfile);
    fclose(atmfile);
    printf("Error creating initialization files\n");
    return 64;
  }
  int bankwrite=fwrite(&key,1,32,bankfile);
  int atmwrite=fwrite(&key,1,32,atmfile);
  fclose(bankfile);
  fclose(atmfile);
  if(bankwrite!=32||atmwrite!=32){
    printf("Error creating initialization files\n");
    return 64;
  }
  printf("Successfully initialized bank state\n");
  return 0;
}
