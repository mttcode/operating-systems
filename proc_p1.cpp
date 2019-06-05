#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <malloc.h>
#include <string.h>
#include <signal.h>

//smernik na FILE
FILE *fp;
//smerniky na debug subory
FILE *e,*o;
//id pipe ako text aj ako int
int pipe_R1_id;
char *R1_id;
//pocitadlo
int i = 0;
//EASTER EGG :-)
int ficovina = 0;
//deklaracia fcii
void odchytenie(int);
void termination(int);

int main(int argc, char *argv[])
{
  e = fopen("proc_p1.err","w");
  o = fopen("proc_p1.out","w");
//ak malo argumentov tak sa skonci
  if(argc < 2)
  {
    fprintf(e,"P1: malo argumentov!\n");
    exit(EXIT_FAILURE);
  }
  else
  {
//inak prirad argumenty
    R1_id = (char *) malloc(sizeof(char)*sizeof(argv[1]));
    if(R1_id == NULL)
    {
      fprintf(e,"P1: malo pamate pre R1_id!\n");
      exit(EXIT_FAILURE);
    }
    else
    {
      R1_id = argv[1];
      pipe_R1_id = atoi(R1_id);
    }
  }
//otvorenie suboru
  fp = fopen("p1.txt","r");
  if(fp == NULL)
  {
    fprintf(e,"P1: Nemozem otvorit p1.txt!\n");
    exit(EXIT_FAILURE);
  }
  else
  {
//tymto v dobe komentovania neviem co som chcel povedat ale budiz je to tu
    ficovina = 1;
  }
//spracovanie signalu
  (void) signal(SIGUSR1,odchytenie);
//nekonecny cyklus nakolko nemam sajnu kedy to ukoncit a ako inak to vyriesit...
//holt pravda je vo hviezdach ked ani srnka netusi.. :D
  while(1)
  {
    pause();
  }
}
//--------------------------------------------------------------------------------------
//definicie fcii
void odchytenie(int sig)
{
  char slovo[200];
  int znak;
//AHA!!! to chcelo byt ze to bude robit len vtedy ked otvori subor
  if(ficovina)
  {
//citanie po riadkoch
    fgets(slovo,200,fp);
//zapis do pipe ako cele slovo
    if(write(pipe_R1_id,slovo,strlen(slovo)) == -1)
    {
      fprintf(e,"P1: write do pipe neuspesny!\n");
      exit(EXIT_FAILURE);
    }
//posli signal materskemu procesu aby sme vedeli ze si one poslal slovo
    kill(getppid(),SIGUSR2);
//ked koniec suboru tak to zavri a utec
    if(slovo == NULL)
    {
      fclose(fp);
      fclose(o);
      fclose(e);
      exit(EXIT_SUCCESS);
    }
  }
}
//ked dostane ^C tak tiez zavri a utec k mamke
void termination(int sig)
{
  fclose(fp);
  fclose(o);
  fclose(e);
  (void) signal(SIGINT,SIG_DFL);
  exit(EXIT_SUCCESS);
}
