#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/types.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <malloc.h>
#include <signal.h>


//smerniky na debug subory
FILE *o,*e;

//deklaracie fcii
void termination(int);

int main(int argc,char *argv[])
{
//pipes a shared memory id
  int pipe_R2,id_S1,id_SM1;
  char *R2,*S1,*SM1;
//adresa shared memory
  void *SHM1 = (void *)0;
//pocitadlo pomarancov
  int i = 0;
//dlzka nacitaneho retazca
  int dlzka = 1;
//ako tak rozmyslam toto mi uz netreba
//  int dlzka;
//  prisiel som na lepsie riesenie ako dookola blizsie takze niektore veci mi uz netreba
  char  *slovo, *shared_stuff/*,*slovo_upr*/;
//union pre semafor
  union semun
  {
    int val;
    struct semid_ds *buf;
    ushort *array;
  } argument;
//sembuf struktura o velkosti 1 -> to som vygooglil nebite ma
  struct sembuf sem_b[1];

  o = fopen("proc_t.out","w");
  e = fopen("proc_t.err","w");

//volanie fcie signal
  (void) signal(SIGINT,termination);

//ak malo argumentov skonci 0> cislo styri je dufam ze dobre :-D
//ano je :-)
//recnicka otazka
  if(argc < 4)
  {
    fprintf(e,"T: malo argumentov!\n");
    exit(EXIT_FAILURE);
  }
  else
  {
//priradovanie argumentov
    R2 = (char *) malloc(sizeof(char)*sizeof(argv[3]));
    if(R2 == NULL)
    {
      fprintf(e,"T: malo pamate pre R2!\n");
      exit(EXIT_FAILURE);
    }
    
    S1 = (char *) malloc(sizeof(char)*sizeof(argv[1]));
    if(S1 == NULL)
    {
      fprintf(e,"T: malo pamate pre S1!\n");
      exit(EXIT_FAILURE);
    }

    SM1 = (char *) malloc(sizeof(char)*sizeof(argv[2]));
    if(SM1 == NULL)
    {
      fprintf(e,"T: malo pamate pre SM1!\n");
      exit(EXIT_FAILURE);
    }

    R2 = argv[3];
    S1 = argv[1];
    SM1 = argv[2];

    pipe_R2 = atoi(R2);
    id_S1 = atoi(S1);
    id_SM1 = atoi(SM1);
  }

  slovo = (char *) malloc(sizeof(char)*150);
  if(slovo == NULL)
  {
    fprintf(e,"T: malo pamate pre slovo!\n");
    exit(EXIT_FAILURE);
  }

//----------------------------------------------------------------
//tuto attachnem memory
  if((SHM1 = shmat(id_SM1,(void *)0,0)) == (void *) -1)
  {
    fprintf(e,"T: shmat zlyhal!\n");
    exit(EXIT_FAILURE);
  }
//premenna typu char v zdielanej pamati (je to odborne presny vyraz toto?)
  shared_stuff = (char *)SHM1;
while(dlzka != 0)
{
//ideme menit hodnotu semaforu
  sem_b[0].sem_num = 0;
  sem_b[0].sem_op = -1;
  sem_b[0].sem_flg = 0;
  if(semop(id_S1,sem_b,1) == -1)
  {
//ak sa nepodari skonci
    fprintf(e,"T: znizenie hodnoty semaforu S1[0] zlyhalo!\n");
    exit(EXIT_FAILURE);
  }

//toto je to elegantnejsie riesenie po znakoch
//predtym nacitavalo po viac znakoch co bola blbost lebo to precitalo vsetko
  i = 0;
  do
  {
    read(pipe_R2,&slovo[i++],1);
  }
  while(slovo[i-1] != '\n');
//nahrad \n koncovym znakom aby to nerobilo blbosti
  memset(shared_stuff,'\0',150);
  slovo[i-1] = '\0';
  strncpy(shared_stuff,slovo,i);
//a proces S dostane zelenu
  sem_b[0].sem_num = 1;
  sem_b[0].sem_op = 1;
  sem_b[0].sem_flg = 0;
  if(semop(id_S1,sem_b,1) == -1)
  {
//ak sa nepodari tak skonci
    fprintf(stderr,"T: zvysenie hodnoty semaforu S1[1] zlyhalo!\n");
    exit(EXIT_FAILURE);
  }
  fprintf(o,"T: %s\n",shared_stuff);
}
  fclose(o);
  fclose(e);
  exit(EXIT_SUCCESS);
}

//definicie fcii
void termination(int sig)
{
  fclose(o);
  fclose(e);
  exit(EXIT_SUCCESS);
}
