#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <sys/sem.h>
#include <malloc.h>
#include <signal.h>
#include <sys/wait.h>
#include <string.h>

//deklaracia odchytenia signalu
void odchytenie(int);
//deklaracia pre odchytenie signalu SIGUSR2 pomocou ktoreho pocitame pocet nacitanych slov zo suborov
void pocitadlo_signalov(int);

//smerniky na debug subory
FILE *o,*e;
//pocitadlo precitanych slov
int pocet_slov = 0;



//main fcia
//-----------------------------------------------------
int main (int argc, char *argv[])
{
  //hlaska na zaciatok :)
  printf("Program trva cca. 20 sekund!!!\n");
  //deklarovanie premennych
  //char portov tcp a udp pre odoslanie procesom
  char *port_tcp, *port_udp;

  //pid procesov pre wait
  int pid_P1, pid_P2, pid_Pr, pid_D, pid_T, pid_S, pid_Serv1, pid_Serv2;

  //pid pre odoslanie procesu Pr ako char
  char *pid_P1_char, *pid_P2_char;

  //pipes
  int pipe_R1[2], pipe_R2[2];

  //id zdielanych pamati
  int id_shm_SM1, id_shm_SM2;

  //id pamati ako char pre odoslanie procesom
  char *shm_SM1, *shm_SM2;

  //id semaforov
  int id_sem_S1, id_sem_S2;

  //id semaforov ako char pre odoslanie procesom
  char *sem_S1, *sem_S2;

  //values pre procesy
  char **values_proc_P1, **values_proc_P2, **values_proc_Pr, **values_proc_T, **values_proc_S, **values_proc_D, **values_proc_Serv1, **values_proc_Serv2;

  //pocitadlo pre for
  int i;

  //id koncov rur ako char pre odoslanie procesom
  char *pipe_R1_citanie, *pipe_R1_pisanie, *pipe_R2_citanie, *pipe_R2_pisanie;

  //status procesov pre waitpid
  int status_P1, status_P2, status_Pr, status_T, status_S, status_D, status_Serv1, status_Serv2;

  //smerniky adries zdielanych pamati
  void *SM1 = (void *)0;
  void *SM2 = (void *)0;
  
  //semunion pre semctl
  union semun
  {
    int val;
    struct semid_ds *buf;
    ushort *array;
  } argument;

  //otvorime si debug subory
  o = fopen("meno.out","w");
  e = fopen("meno.err","w");

  //pocet argumentov ma byt tri
  //1.)nazov programu
  //2.)port tcp
  //3.)port udp
  if ((argc <= 1) || (argc > 3))
  {
    fprintf(e,"Nespravny pocet parametrov: spusti v tvare ./zadanie port_tcp port_udp !!!\n");
    exit(EXIT_FAILURE);
  }
  else
  {
    port_tcp = (char *) malloc(sizeof(argv[1])*sizeof(char));
    if (port_tcp == NULL)
    {
      fprintf(e,"Malo pamate pre port_tcp\n");
      exit(EXIT_FAILURE);
    }
    if ((port_udp = (char *) malloc(sizeof(argv[2])*sizeof(char))) == NULL)
    {
      fprintf(e,"Malo pamate pre port_udp\n");
      exit(EXIT_FAILURE);
    }
    //priradenie hodnot argumentov
    port_tcp = argv[1];
    port_udp = argv[2];
    fprintf(o,"Priradenie portov uspesne!\n");
  }

//---------------------------------------------------------------------------
//inicializacia shared memory 1
  if ((id_shm_SM1 = shmget ((key_t) 10001, 150, 0666 | IPC_CREAT)) == -1)
  {
    fprintf(e,"shmget pre SM1 zlyhal\n");
    exit(EXIT_FAILURE);
  }
  else
  {
    SM1 = shmat(id_shm_SM1,NULL,0);
    if (SM1 == NULL)
    {
      fprintf(e,"Nemozem pridelit pamat pre SM1!\n");
      exit(EXIT_FAILURE);
    }
    else
    {
      fprintf(o,"Priradenie SM1 uspesne!\n");
    }
  }

//---------------------------------------------------------------------------
//inicializacia shared memory 2
  if ((id_shm_SM2 = shmget ((key_t) 10002, 150, 0666 | IPC_CREAT)) == -1)
  {
    fprintf(e,"shmget pre SM2 zlyhal\n");
    exit(EXIT_FAILURE);
  }
  else
  {
    SM2 = shmat(id_shm_SM2,NULL,0);
    if(SM2 == NULL)
    {
      fprintf(e,"Nemozem pridelit pamat pre SM2!\n");
      exit(EXIT_FAILURE);
    }
    else
    {
      fprintf(o,"Priradenie SM2 uspesne!\n");
    }
  }

//---------------------------------------------------------------------------
//inicializaica prveho pipeu
  if (pipe(pipe_R1) == -1)
  {
    fprintf(e,"Nemozem vytvorit pipe pre pipe_R1\n");
    exit(EXIT_FAILURE);
  }
  else
  {
    fprintf(o,"Vytvorenie pipe 1 uspesne!\n");
  }

//---------------------------------------------------------------------------
//inicializacia druheho pipeu
  if (pipe(pipe_R2) == -1)
  {
    fprintf(e,"Nemozem vytvorit pipe pre pipe_R2\n");
    exit(EXIT_FAILURE);
  }
  else
  {
    fprintf(o,"Vytvorenie pipe 2 uspesne!\n");
  }

//---------------------------------------------------------------------------
//inicializacia semaforu jedna
  if ((id_sem_S1 = semget((key_t) 10003, 2, 0666 | IPC_CREAT)) == -1)
  {
    fprintf(e,"Nemozem inicializovat semafor 1\n");
    exit(EXIT_FAILURE);
  }
  else
  {
    argument.val = 0;
    if(semctl(id_sem_S1,1,SETVAL,argument) < 0)
    {
      fprintf(e,"Nemozem nastavit hodnotu S1[1] na 0!\n");
      exit(EXIT_FAILURE);
    }
    else
    {
      fprintf(o,"Hodnota semaforu S1[1] nastavena uspesne!\n");
    }
    argument.val = 1;
    if (semctl(id_sem_S1,0,SETVAL,argument) < 0)
    {
     fprintf(e,"Nemozem nastavit hodnotu S1[0] na 1!\n");
     exit(EXIT_FAILURE);
    }
    else
    {
      fprintf(o,"Hodnota semaforu S1[0] nastavena uspesne!\n");
    }
  }

//---------------------------------------------------------------------------
//inicializacia semaforu dva
  if ((id_sem_S2 = semget((key_t) 10004, 2, 0666 | IPC_CREAT)) == -1)
  {
    fprintf(e,"Nemozem inicializovat semafor 2\n");
    exit(EXIT_FAILURE);
  }
  else
  {
    argument.val = 1;
    if(semctl(id_sem_S2,0,SETVAL,argument) < 0)
    {
      fprintf(e,"Nemozem nastavit hodnotu S2[0] na 1!\n");
      exit(EXIT_FAILURE);
    }
    else
    {
      fprintf(o,"Hodnota semaforu S2[0] nastavena uspesne!\n");
    }
    argument.val = 0;
    if(semctl(id_sem_S2,1,SETVAL,argument) < 0)
    {
      fprintf(e,"Nemozem nastavit hodnotu S2[1] na 0!\n");
      exit(EXIT_FAILURE);
    }
    else
    {
      fprintf(o,"Hodnota semaforu S2[1] nastavena uspesne!\n");
    }
  }

//koniec inicializacii
//---------------------------------------------------------------------------

//volanie fcie signal
  (void) signal(SIGUSR1,odchytenie);
  (void) signal(SIGUSR2,pocitadlo_signalov);

/*najprv spustime P1 a potom P2
 *potom spustame PR
 *ked sa PR skonci mozeme spustit dalsie procesy
 *je to z toho dovodu aby sme nespustili proces serv2 skorej ako skonci PR s prijimanim slov
 *lebo inac by ani dear God nevedel kolko slov vlastne nacital
*/

//---------------------------------------------------------------------------
//priprava argumentov pre proces proc_p1
  pipe_R1_pisanie = (char *) malloc(10*sizeof(char));
  if (pipe_R1_pisanie == NULL)
  {
    fprintf(e,"Malo pamate pre pipe_R1_pisanie!\n");
    exit(EXIT_FAILURE);
  }

  if((sprintf(pipe_R1_pisanie,"%i",pipe_R1[1])) < 0)
  {
    fprintf(e,"Sprintf pre pipe_R1_pisanie sa nepodaril!\n");
    exit(EXIT_FAILURE);
  }
  else
  {
    fprintf(o,"pipe_R1_pisanie = %s s dlzkou retazca %i\n",pipe_R1_pisanie,strlen(pipe_R1_pisanie));
  }

  values_proc_P1 = (char **) malloc(3*sizeof(char *));
  if(values_proc_P1 == NULL)
  {
    fprintf(e,"Malo pamate pre values of P1!\n");
    exit(EXIT_FAILURE);
  }

  values_proc_P1[0] = (char *) malloc(sizeof(char)*sizeof("proc_p1"));
  if(values_proc_P1[0] == NULL)
  {
    fprintf(e,"Malo pamate pre values[0] of P1!\n");
    exit(EXIT_FAILURE);
  }

  values_proc_P1[1] = (char *) malloc(sizeof(char)*sizeof(pipe_R1_pisanie));
  if(values_proc_P1[1] == NULL)
  {
    fprintf(e,"Malo pamate pre values[1] of P1!\n");
    exit(EXIT_FAILURE);
  }

  values_proc_P1[2] = (char *) malloc(sizeof(char)*sizeof(char *));
  if(values_proc_P1[2] == NULL)
  {
    fprintf(e,"Malo pamate pre values of P1!\n");
    exit(EXIT_FAILURE);
  }

  values_proc_P1[0] = "proc_p1";
  values_proc_P1[1] = pipe_R1_pisanie;
  values_proc_P1[2] = (char *)0;

  pid_P1 = fork();
  switch(pid_P1)
  {
    case -1:
      fprintf(e,"Nepodarilo sa inicializovat fork pre proces P1!\n");
      exit(EXIT_FAILURE);
      break;
    case 0:
      execve("proc_p1",values_proc_P1,0);
      exit(EXIT_SUCCESS);
      break;
  }
  usleep(250);
//---------------------------------------------------------------------------
//priprava argumentov pre proces proc_p2
  values_proc_P2 = (char **) malloc(3*sizeof(char *));
  if(values_proc_P2 == NULL)
  {
    fprintf(e,"Malo pamate pre values of P2!\n");
    exit(EXIT_FAILURE);
  }

  values_proc_P2[0] = (char *) malloc(sizeof(char)*sizeof("proc_p2"));
  if(values_proc_P2[0] == NULL)
  {
    fprintf(e,"Malo pamate pre values[0] of P2!\n");
    exit(EXIT_FAILURE);
  }

  values_proc_P2[1] = (char *) malloc(sizeof(char)*sizeof(pipe_R1_pisanie));
  if(values_proc_P2[1] == NULL)
  {
    fprintf(e,"Malo pamate pre values[1] of P2!\n");
    exit(EXIT_FAILURE);
  }

  values_proc_P2[2] = (char *) malloc(sizeof(char)*sizeof(char *));
  if(values_proc_P2[2] == NULL)
  {
    fprintf(e,"Malo pamate pre values[2] of P2!\n");
    exit(EXIT_FAILURE);
  }

  values_proc_P2[0] = "proc_p2";
  values_proc_P2[1] = pipe_R1_pisanie;
  values_proc_P2[2] = (char *)0;

  pid_P2 = fork();
  switch(pid_P2)
  {
    case -1:
      fprintf(e,"Nepodarilo sa inicializovat fork pre proces P2!\n");
      exit(EXIT_FAILURE);
      break;
    case 0:
      execve("proc_p2",values_proc_P2,0);
      exit(EXIT_SUCCESS);
      break;
  }

  usleep(250);
//---------------------------------------------------------------------------
//priprava argumentov pre proces proc_pr
  pid_P1_char = (char *) malloc(10*sizeof(char));
  if (pid_P1_char == NULL)
  {
    fprintf(e,"Malo pamate pre pid_P1_char!\n");
    exit(EXIT_FAILURE);
  }

  pid_P2_char = (char *) malloc(10*sizeof(char));
  if (pid_P2_char == NULL)
  {
    fprintf(e,"Malo pamate pre pid_P2_char!\n");
    exit(EXIT_FAILURE);
  }

  pipe_R1_citanie = (char *) malloc(10*sizeof(char));
  if(pipe_R1_citanie == NULL)
  {
    fprintf(e,"Malo pamate pre pipe_R1_citanie!\n");
    exit(EXIT_FAILURE);
  }

  pipe_R2_pisanie = (char *) malloc(10*sizeof(char));
  if(pipe_R2_pisanie == NULL)
  {
    fprintf(e,"Malo pamate pre pipe_R2_citanie!\n");
    exit(EXIT_FAILURE);
  }

  if((sprintf(pid_P1_char,"%i",pid_P1)) < 0)
  {
    fprintf(e,"Sprintf pre pid P1 char sa nepodaril!\n");
    exit(EXIT_FAILURE);
  }
  else
  {
    fprintf(o,"pid_P1_char = %s s dlzkou %i\n",pid_P1_char,strlen(pid_P1_char));
  }

  if((sprintf(pid_P2_char,"%i",pid_P2)) < 0)
  {
    fprintf(e,"Sprintf pre pid P2 char sa nepodaril!\n");
    exit(EXIT_FAILURE);
  }
  else
  {
    fprintf(o,"pid_P2_char = %s s dlzkou %i\n",pid_P2_char,strlen(pid_P2_char));
  }

  if((sprintf(pipe_R1_citanie,"%i",pipe_R1[0])) < 0)
  {
    fprintf(e,"Sprintf pre pipe R1 citanie sa nepodaril!\n");
    exit(EXIT_FAILURE);
  }
  else
  {
    fprintf(o,"pipe_R1_citanie = %s s dlzkou %i\n",pipe_R1_citanie,strlen(pipe_R1_citanie));
  }

  if((sprintf(pipe_R2_pisanie,"%i",pipe_R2[1])) < 0)
  {
    fprintf(e,"Sprintf pre pipe R2 pisanie sa nepodaril!\n");
    exit(EXIT_FAILURE);
  }
  else
  {
    fprintf(o,"pipe_R2_pisanie = %s s dlzkou %i\n",pipe_R2_pisanie,strlen(pipe_R2_pisanie));
  }

  values_proc_Pr = (char **) malloc(6*sizeof(char *));
  if(values_proc_Pr == NULL)
  {
    fprintf(e,"Malo pamate pre values of Pr!\n");
    exit(EXIT_FAILURE);
  }

  values_proc_Pr[0] = (char *) malloc(sizeof(char)*sizeof("proc_pr"));
  if(values_proc_Pr[0] == NULL)
  {
    fprintf(e,"Malo pamate pre values[0] of Pr!\n");
    exit(EXIT_FAILURE);
  }

  values_proc_Pr[1] = (char *) malloc(sizeof(char)*sizeof(pid_P1_char));
  if(values_proc_Pr[1] == NULL)
  {
    fprintf(e,"Malo pamate pre values[1] of Pr!\n");
    exit(EXIT_FAILURE);
  }

  values_proc_Pr[2] = (char *) malloc(sizeof(char)*sizeof(pid_P2_char));
  if(values_proc_Pr[2] == NULL)
  {
    fprintf(e,"Malo pamate pre values[2] of Pr!\n");
    exit(EXIT_FAILURE);
  }

  values_proc_Pr[3] = (char *) malloc(sizeof(char)*sizeof(pipe_R1_citanie));
  if(values_proc_Pr[3] == NULL)
  {
    fprintf(e,"Malo pamate pre values[3] of Pr!\n");
    exit(EXIT_FAILURE);
  }

  values_proc_Pr[4] = (char *) malloc(sizeof(char)*sizeof(pipe_R2_pisanie));
  if(values_proc_Pr[4] == NULL)
  {
    fprintf(e,"Malo pamate pre values[4] of Pr!\n");
    exit(EXIT_FAILURE);
  }

  values_proc_Pr[5] = (char *) malloc(sizeof(char)*sizeof(char *));
  if(values_proc_Pr[5] == NULL)
  {
    fprintf(e,"Malo pamate pre values[5] of Pr!\n");
    exit(EXIT_FAILURE);
  }

  values_proc_Pr[0] = "proc_pr";
  values_proc_Pr[1] = pid_P1_char;
  values_proc_Pr[2] = pid_P2_char;
  values_proc_Pr[3] = pipe_R1_citanie;
  values_proc_Pr[4] = pipe_R2_pisanie;
  values_proc_Pr[5] = (char *)0;

  pid_Pr = fork();
  switch(pid_Pr)
  {
    case -1:
      fprintf(e,"Nepodarilo sa inicializovat fork pre proces Pr!\n");
      exit(EXIT_FAILURE);
      break;
    case 0:
      execve("proc_pr",values_proc_Pr,0);
      exit(EXIT_SUCCESS);
      break;
  }

//pockame si kym skonci PR aby sme spustili dalsie
//a sucasne killneme tie dva nacitavajuce one procesy
  waitpid(pid_Pr,&status_Pr,0);
  if (WIFEXITED(status_Pr))
  {
    fprintf(o,"Pr skoncil status = %i\n", WEXITSTATUS(status_Pr));
    kill(pid_P1,SIGINT);
    kill(pid_P2,SIGINT);
  }

//---------------------------------------------------------------------------
//priprava argumentov pre proces proc_t
  pipe_R2_citanie = (char *) malloc(sizeof(char)*10);
  if(pipe_R2_citanie == NULL)
  {
    fprintf(e,"Malo pamate pre pipe R2 citanie!\n");
    exit(EXIT_FAILURE);
  }

  sem_S1 = (char *) malloc(sizeof(char)*10);
  if(sem_S1 == NULL)
  {
    fprintf(e,"Malo pamate pre semafor S1!\n");
    exit(EXIT_FAILURE);
  }

  shm_SM1 = (char *) malloc(sizeof(char)*10);
  if(shm_SM1 == NULL)
  {
    fprintf(e,"Malo pamate pre shared mem SM1!\n");
    exit(EXIT_FAILURE);
  }

  if((sprintf(pipe_R2_citanie,"%i",pipe_R2[0])) < 0)
  {
    fprintf(e,"Sprintf pre pipe R2 citanie zlyhal!\n");
    exit(EXIT_FAILURE);
  }
  else
  {
    fprintf(o,"pipe_R2_citanie = %s s dlzkou %i\n",pipe_R2_citanie,strlen(pipe_R2_citanie));
  }

  if((sprintf(sem_S1,"%i",id_sem_S1)) < 0)
  {
    fprintf(e,"Sprintf pre sem_S1 zlyhal!\n");
    exit(EXIT_FAILURE);
  }
  else
  {
    fprintf(o,"sem_S1 = %s s dlzkou %i\n",sem_S1,strlen(sem_S1));
  }

  if((sprintf(shm_SM1,"%i",id_shm_SM1)) < 0)
  {
    fprintf(e,"Sprintf pre shm_SM1 zlyhal!\n");
    exit(EXIT_FAILURE);
  }
  else
  {
    fprintf(o,"shm_SM1 = %s s dlzkou %i\n",shm_SM1,strlen(shm_SM1));
  }

  values_proc_T = (char **) malloc(5*sizeof(char *));
  if(values_proc_T == NULL)
  {
    fprintf(e,"Malo pamate pre values_proc_T!\n");
    exit(EXIT_FAILURE);
  }

  values_proc_T[0] = (char *) malloc(sizeof(char)*sizeof("proc_t"));
  if(values_proc_T[0] == NULL)
  {
    fprintf(e,"Malo pamate pre values[0] of T!\n");
    exit(EXIT_FAILURE);
  }

  values_proc_T[1] = (char *) malloc(sizeof(char)*sizeof(sem_S1));
  if(values_proc_T[1] == NULL)
  {
    fprintf(e,"Malo pamate pre values[1] of T!\n");
    exit(EXIT_FAILURE);
  }

  values_proc_T[2] = (char *) malloc(sizeof(char)*sizeof(shm_SM1));
  if(values_proc_T[2] == NULL)
  {
    fprintf(e,"Malo pamate pre values[2] of T!\n");
    exit(EXIT_FAILURE);
  }

  values_proc_T[3] = (char *) malloc(sizeof(char)*sizeof(pipe_R2_citanie));
  if(values_proc_T[3] == NULL)
  {
    fprintf(e,"Malo pamate pre values[3] of T!\n");
    exit(EXIT_FAILURE);
  }

  values_proc_T[4] = (char *) malloc(sizeof(char)*sizeof(char *));
  if(values_proc_T[4] == NULL)
  {
    fprintf(e,"Malo pamate pre values[0] of T!\n");
    exit(EXIT_FAILURE);
  }

  values_proc_T[0] = "proc_t";
  values_proc_T[1] = sem_S1;
  values_proc_T[2] = shm_SM1;
  values_proc_T[3] = pipe_R2_citanie;
  values_proc_T[4] = (char *)0;

  pid_T = fork();
  switch(pid_T)
  {
    case -1:
      fprintf(e,"Nepodarilo sa incializovat fork pre proces T!\n");
      exit(EXIT_FAILURE);
      break;
    case 0:
      execve("proc_t",values_proc_T,0);
      exit(EXIT_SUCCESS);
      break;
  }

//---------------------------------------------------------------------------
//priprava argumentov pre proces proc_s
  shm_SM2 = (char *) malloc(10*sizeof(char));
  if(shm_SM2 == NULL)
  {
    fprintf(e,"Malo pamate pre shm_SM2!\n");
    exit(EXIT_FAILURE);
  }

  sem_S2 = (char *) malloc(10*sizeof(char));
  if(sem_S2 == NULL)
  {
    fprintf(e,"Malo pamate pre sem_S2!\n");
    exit(EXIT_FAILURE);
  }
  
  if((sprintf(sem_S2,"%i",id_sem_S2)) < 0)
  {
    fprintf(e,"Sprintf pre sem S2 sa nepdoaril!\n");
    exit(EXIT_FAILURE);
  }
  else
  {
    fprintf(o,"sem_S2 = %s s dlzkou %i\n",sem_S2,strlen(sem_S2));
  }

  if((sprintf(shm_SM2,"%i",id_shm_SM2)) < 0)
  {
    fprintf(e,"Sprintf pre shm SM2 sa nepdoaril!\n");
    exit(EXIT_FAILURE);
  }
  else
  {
    fprintf(o,"shm_SM2 = %s s dlzkou %i\n",shm_SM2,strlen(shm_SM2));
  }

  values_proc_S = (char **) malloc(6*sizeof(char *));
  if(values_proc_S == NULL)
  {
    fprintf(e,"Malo pamate pre values of S!\n");
    exit(EXIT_FAILURE);
  }
  
  values_proc_S[0] = (char *) malloc(sizeof(char)*sizeof("proc_s"));
  if(values_proc_S[0] == NULL)
  {
    fprintf(e,"Malo pamate pre values[0] of S!\n");
    exit(EXIT_FAILURE);
  }
  
  values_proc_S[1] = (char *) malloc(sizeof(char)*sizeof(shm_SM1));
  if(values_proc_S[1] == NULL)
  {
    fprintf(e,"Malo pamate pre values[1] of S!\n");
    exit(EXIT_FAILURE);
  }
  
  values_proc_S[2] = (char *) malloc(sizeof(char)*sizeof(sem_S1));
  if(values_proc_S[2] == NULL)
  {
    fprintf(e,"Malo pamate pre values[2] of S!\n");
    exit(EXIT_FAILURE);
  }
  
  values_proc_S[3] = (char *) malloc(sizeof(char)*sizeof(shm_SM2));
  if(values_proc_S[3] == NULL)
  {
    fprintf(e,"Malo pamate pre values[3] of S!\n");
    exit(EXIT_FAILURE);
  }
  
  values_proc_S[4] = (char *) malloc(sizeof(char)*sizeof(sem_S2));
  if(values_proc_S[4] == NULL)
  {
    fprintf(e,"Malo pamate pre values[4] of S!\n");
    exit(EXIT_FAILURE);
  }
  
  values_proc_S[5] = (char *) malloc(sizeof(char)*sizeof(char *));
  if(values_proc_S[5] == NULL)
  {
    fprintf(e,"Malo pamate pre values[5] of S!\n");
    exit(EXIT_FAILURE);
  }
  
  values_proc_S[0] = "proc_s";
  values_proc_S[1] = shm_SM1;
  values_proc_S[2] = sem_S1;
  values_proc_S[3] = shm_SM2;
  values_proc_S[4] = sem_S2;
  values_proc_S[5] = (char *)0;

  pid_S = fork();
  switch(pid_S)
  {
    case -1:
      fprintf(e,"Nepodarilo sa inicializovat fork pre proces S!\n");
      exit(EXIT_FAILURE);
      break;
    case 0:
      execve("proc_s",values_proc_S,0);
      exit(EXIT_SUCCESS);
      break;
  }


//---------------------------------------------------------------------------
//priprava argumentov pre proces proc_serv2
  values_proc_Serv2 = (char **) malloc(4*sizeof(char *));
  if(values_proc_Serv2 == NULL)
  {
    fprintf(e,"Malo pamate pre values of Serv2!\n");
    exit(EXIT_FAILURE);
  }
  
  values_proc_Serv2[0] = (char *) malloc(sizeof(char)*sizeof("proc_serv2"));
  if(values_proc_Serv2[0] == NULL)
  {
    fprintf(e,"Malo pamate pre values[0] of Serv2!\n");
    exit(EXIT_FAILURE);
  }
  
  values_proc_Serv2[1] = (char *) malloc(sizeof(char)*sizeof(port_udp));
  if(values_proc_Serv2[1] == NULL)
  {
    fprintf(e,"Malo pamate pre values[1] of Serv2!\n");
    exit(EXIT_FAILURE);
  }
  
  values_proc_Serv2[2] = (char *) malloc(sizeof(char)*sizeof(pocet_slov));
  if(values_proc_Serv2[2] == NULL)
  {
    fprintf(e,"Malo pamate pre values[2] of Serv2!\n");
    exit(EXIT_FAILURE);
  }
  
  values_proc_Serv2[3] = (char *) malloc(sizeof(char)*sizeof(char *));
  if(values_proc_Serv2[3] == NULL)
  {
    fprintf(e,"Malo pamate pre values[3] of Serv2!\n");
    exit(EXIT_FAILURE);
  }
  
  if(sprintf(values_proc_Serv2[2],"%i",pocet_slov) < 0)
  {
    fprintf(e,"sprintf pre pocet slov sa nepodaril!\n");
    exit(EXIT_FAILURE);
  }
  values_proc_Serv2[0] = "proc_serv2";
  values_proc_Serv2[1] = port_udp;
  values_proc_Serv2[3] = (char *)0;

  pid_Serv2 = fork();
  switch(pid_Serv2)
  {
    case -1:
      fprintf(e,"Nepodarilo sa inicializovat fork pre proces Serv2!\n");
      exit(EXIT_FAILURE);
      break;
    case 0:
      execve("proc_serv2",values_proc_Serv2,0);
      exit(EXIT_SUCCESS);
      break;
  }
  sleep(2);


//---------------------------------------------------------------------------
//priprava argumentov pre proces proc_serv1
  values_proc_Serv1 = (char **) malloc(4*sizeof(char *));
  if(values_proc_Serv1 == NULL)
  {
    fprintf(e,"Malo pamate pre values of Serv1!\n");
    exit(EXIT_FAILURE);
  }
  
  values_proc_Serv1[0] = (char *) malloc(sizeof(char)*sizeof("proc_serv1"));
  if(values_proc_Serv1[0] == NULL)
  {
    fprintf(e,"Malo pamate pre values[0] of Serv1!\n");
    exit(EXIT_FAILURE);
  }
  
  values_proc_Serv1[1] = (char *) malloc(sizeof(char)*sizeof(port_tcp));
  if(values_proc_Serv1[1] == NULL)
  {
    fprintf(e,"Malo pamate pre values[1] of Serv1!\n");
    exit(EXIT_FAILURE);
  }
  
  values_proc_Serv1[2] = (char *) malloc(sizeof(char)*sizeof(port_udp));
  if(values_proc_Serv1[2] == NULL)
  {
    fprintf(e,"Malo pamate pre values[2] of Serv1!\n");
    exit(EXIT_FAILURE);
  }
  
  values_proc_Serv1[3] = (char *) malloc(sizeof(char)*sizeof(char *));
  if(values_proc_Serv1[3] == NULL)
  {
    fprintf(e,"Malo pamate pre values[3] of Serv1!\n");
    exit(EXIT_FAILURE);
  }
  
  values_proc_Serv1[0] = "proc_serv1";
  values_proc_Serv1[1] = port_tcp;
  values_proc_Serv1[2] = port_udp;
  values_proc_Serv1[3] = (char *)0;

  pid_Serv1 = fork();
  switch(pid_Serv1)
  {
    case -1:
      fprintf(e,"Nepodarilo sa inicializovat fork pre proces Serv1!\n");
      exit(EXIT_FAILURE);
      break;
    case 0:
      execve("proc_serv1",values_proc_Serv1,0);
      exit(EXIT_SUCCESS);
      break;
  }
  sleep(2);


//---------------------------------------------------------------------------
//priprava argumentov pre proces proc_d
  values_proc_D = (char **) malloc(5*sizeof(char *));
  if(values_proc_D == NULL)
  {
    fprintf(e,"Malo pamate pre values of D!\n");
    exit(EXIT_FAILURE);
  }
  
  values_proc_D[0] = (char *) malloc(sizeof(char)*sizeof("proc_d"));
  if(values_proc_D[0] == NULL)
  {
    fprintf(e,"Malo pamate pre values[0] of D!\n");
    exit(EXIT_FAILURE);
  }
  
  values_proc_D[1] = (char *) malloc(sizeof(char)*sizeof(shm_SM2));
  if(values_proc_D[1] == NULL)
  {
    fprintf(e,"Malo pamate pre values[0] of D!\n");
    exit(EXIT_FAILURE);
  }
  
  values_proc_D[2] = (char *) malloc(sizeof(char)*sizeof(sem_S2));
  if(values_proc_D[2] == NULL)
  {
    fprintf(e,"Malo pamate pre values[0] of D!\n");
    exit(EXIT_FAILURE);
  }
  
  values_proc_D[3] = (char *) malloc(sizeof(char)*sizeof(port_tcp));
  if(values_proc_D[3] == NULL)
  {
    fprintf(e,"Malo pamate pre values[0] of D!\n");
    exit(EXIT_FAILURE);
  }
  
  values_proc_D[4] = (char *) malloc(sizeof(char)*sizeof(char *));
  if(values_proc_D[4] == NULL)
  {
    fprintf(e,"Malo pamate pre values[0] of D!\n");
    exit(EXIT_FAILURE);
  }
  
  values_proc_D[0] = "proc_d";
  values_proc_D[1] = shm_SM2;
  values_proc_D[2] = sem_S2;
  values_proc_D[3] = port_tcp;
  values_proc_D[4] = (char *)0;

  pid_D = fork();
  switch(pid_D)
  {
    case -1:
      fprintf(e,"Nepodarilo sa inicializovat fork pre proces D!\n");
      exit(EXIT_FAILURE);
      break;
    case 0:
      execve("proc_d",values_proc_D,0);
      exit(EXIT_SUCCESS);
      break;
  }


//---------------------------------------------------------------------------
//cakame kym udp server skonci svoju cinnost
//a killneme secky one procesy
  waitpid(pid_Serv2,&status_Serv2,0);
  if (WIFEXITED(status_Serv2))
  {
    fprintf(o,"Serv2 skoncil status = %i\n", WEXITSTATUS(status_Serv2));
    kill(pid_T,SIGINT);
    kill(pid_S,SIGINT);
    kill(pid_D,SIGINT);
    kill(pid_Serv1,SIGINT);
  }


//---------------------------------------------------------------------------
  semctl(id_sem_S1,1,IPC_RMID,argument);
  semctl(id_sem_S1,0,IPC_RMID,argument);
  semctl(id_sem_S2,1,IPC_RMID,argument);
  semctl(id_sem_S2,0,IPC_RMID,argument);

//---------------------------------------------------------------------------
//zmazanie pamati
//ostrenie chyb nie je a nateraz nebude
  shmdt(SM1);
  shmctl((key_t)10001,IPC_RMID,0);
  shmdt(SM2);
  shmctl((key_t)10002,IPC_RMID,0);

  exit(EXIT_SUCCESS);
}

//---------------------------------------------------------------------------
//signal handler
void odchytenie(int sig)
{
  fprintf(o,"Proces pripraveny!\n");
}

//pocitanie kolko slov si vyziada PR
void pocitadlo_signalov(int sig)
{
  pocet_slov++;
}
