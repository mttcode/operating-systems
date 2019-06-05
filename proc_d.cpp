#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <malloc.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <signal.h>

//smerniky na debug subory
FILE *e,*o;
//id socketu
int TCP_socket;
//deklaracia fcie
void termination(int);

//hlavna fcia
int main(int argc, char *argv[])
{
  //id shared memory
  int id_shm2;
  char *shm2;
  //id semaforu2
  int id_sem2;
  char *sem2;
  //port TCP
  int port_tcp;
  char *tcp;
  //toto bude zrejme smernik na adresu SM
  void *shm_addr = (void *) 0;
  //buffer a premenna(?) v zdielanej pamati
  char *shared_stuff,*zasobnik;
  //struktura ku semaforu
  struct sembuf sem_b[1];
  //toto bude nejake pocitadlo
  int i = 0;
  //dlzka nacitaneho slova
  int dlzka = 1;
  //struktura k tym sietovym veciam
  struct sockaddr_in server_address;
  //navratova hodnota connection
  int conn;

  (void) signal(SIGINT,termination);
  o = fopen("proc_d.out","w");
  e = fopen("proc_d.err","w");
//-----------------------------------------------------------
//ak je malo argumentov tak skonci
  if(argc < 4)
  {
    fprintf(e,"Obdrzal som malo parametrov!\n");
    exit(EXIT_FAILURE);
  }
  else
  {
    shm2 = (char *) malloc(sizeof(char)*sizeof(argv[1]));
    if(shm2 == NULL)
    {
      fprintf(e,"malo pamate pre shm2!\n");
      exit(EXIT_FAILURE);
    }

    sem2 = (char *) malloc(sizeof(char)*sizeof(argv[2]));
    if (sem2 == NULL)
    {
      fprintf(e,"malo pamate pre sem2!\n");
      exit(EXIT_FAILURE);
    }

    tcp = (char *) malloc(sizeof(char)*sizeof(argv[3]));
    if(tcp == NULL)
    {
      fprintf(e,"malo pamate pre tcp!\n");
      exit(EXIT_FAILURE);
    }
    shm2 = argv[1];
    sem2 = argv[2];
    tcp = argv[3];

    id_shm2 = atoi(shm2);
    id_sem2 = atoi(sem2);
    port_tcp = atoi(tcp);
  }

//pripojenie shared memory
  if((shm_addr = shmat(id_shm2,(void *)0,0)) == (void *) -1)
  {
    fprintf(e,"smhat zlyhal!\n");
    exit(EXIT_FAILURE);
  }
//shared_stuff priradime mu pekne adresu pamate
  shared_stuff = (char *)shm_addr;
  zasobnik = (char *) malloc(sizeof(char)*150);

//vytvorime si socket pre tcp spojenie
  TCP_socket = socket(AF_INET, SOCK_STREAM, 0);
  if(TCP_socket == -1)
  {
    fprintf(e,"socket neuspesny!\n");
    exit(EXIT_FAILURE);
  }
//nulovanie server address
  bzero((char *) &server_address,sizeof(server_address));
  server_address.sin_family = AF_INET;
  server_address.sin_port = htons(port_tcp);
  server_address.sin_addr.s_addr = inet_addr("127.0.0.1");
//a pripojenie
  conn = connect(TCP_socket,(struct sockaddr*)&server_address,sizeof(server_address));
  if(conn == -1)
  {
    fprintf(e,"neviem sa pripojit na server!\n");
    close(EXIT_FAILURE);
  }

//ta dlzka povodne chcela byt ze ked nacita nula znakov ale neskor som to spravil inak lebo tak to neslo
  while(dlzka != 0)
  {
//hodnota semaforu zmenena
    sem_b[0].sem_num = 1;
    sem_b[0].sem_op = -1;
    sem_b[0].sem_flg = 0;
    if(semop(id_sem2,sem_b,1) == -1)
    {
      fprintf(e,"znizenie hodnoty semaforu S2[1] zlyhalo!\n");
      exit(EXIT_FAILURE);
    }
//nuluj zasobnik
    bzero(zasobnik,150);
//nakopiruj tam obsah v zdielanej pamati
    strcpy(zasobnik,shared_stuff);
    fprintf(o,"%s\n",zasobnik);
//nastavenie SM aby sme tam mali pekne vela znakov ze novy riadok
    memset(shared_stuff,'\n',150);
//a zmenime semafor
    sem_b[0].sem_num = 0;
    sem_b[0].sem_op = 1;
    sem_b[0].sem_flg = 0;
    if(semop(id_sem2,sem_b,1) == -1)
    {
      fprintf(e,"zvysenie hodnoty semaforu S2[0] zlyhalo!\n");
      exit(EXIT_FAILURE);
    }

//odosleme slovo na server
    send(TCP_socket,zasobnik,150,0);
//ideme do hajan si oddychnut lebo vela prace toto :D
    sleep(1);
  }
  //koniec -> tymto sposobom aj tak ten koniec nejde lebo dlzka v SM je stale 150 znakov
  close(TCP_socket);
  fclose(o);
  fclose(e);
  exit(EXIT_SUCCESS);
}

//definicia fcie
void termination(int sig)
{
  fclose(o);
  fclose(e);
  close(TCP_socket);
  (void) signal(SIGINT,SIG_DFL);
  exit(EXIT_SUCCESS);
}
