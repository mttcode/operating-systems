#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <malloc.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <signal.h>

//deskriptor na subor serv2.txt
int subor_fd;
//smerniky na debug subory
FILE *e,*o;
//id(?) UDP socketu
int UDP_socket;

//main fcia
int main(int argc, char *argv[])
{
//cislo udp portu
  int port_udp;
  char *udp;
//navratova hodnota connect
  int connection;
//slovo na zapis
  char *slovo;
//struktura k netovym veciam
  struct sockaddr_in server_address;
//pardon ale uz ma stvalo ze to furt neslo
  int prd = 0;
//pocitadlo
  int pocitadlo;
  char *pocitadlo_char;

  o = fopen("proc_serv2.out","w");
  e = fopen("proc_serv2.err","w");

//ak si dostal malo argumentov tak sa vypni
  if (argc < 3)
  {
    fprintf(e,"malo argumentov!\n");
    exit(EXIT_FAILURE);
  }
  else
  {
    udp = (char *) malloc(sizeof(char)*sizeof(argv[1]));
    if (udp == NULL)
    {
      fprintf(e,"malo pamate pre udp!\n");
      exit(EXIT_FAILURE);
    }
    pocitadlo_char = (char *) malloc(sizeof(char)*sizeof(argv[2]));
    if(pocitadlo_char == NULL)
    {
      fprintf(e,"malo pamate per pocitadlo_char\n");
      exit(EXIT_FAILURE);
    }
      pocitadlo_char = argv[2];
      pocitadlo = atoi(pocitadlo_char);
      udp = argv[1];
      port_udp = atoi(udp);
  }
//otvorime si subor na zapis vysledkov
  subor_fd = open("serv2.txt", O_CREAT | O_WRONLY, S_IWUSR | S_IRUSR | S_IRGRP | S_IROTH);
  if (subor_fd == -1)
  {
    fprintf(e,"nemozem otvorit serv2.txt!\n");
    exit(EXIT_FAILURE);
  }
//nadefinovanie spojenia
  server_address.sin_family = AF_INET;
  server_address.sin_addr.s_addr = INADDR_ANY;
  server_address.sin_port = htons(port_udp);
//vytvorenie socketu
  UDP_socket = socket(AF_INET,SOCK_DGRAM,0);
  if(UDP_socket == -1)
  {
    fprintf(e,"neviem vytvorit socket!\n");
    exit(EXIT_FAILURE);
  }
  else
  {
//to pretypovanie na char nefunguje hodi tam null ale aspon viem ze ten blby socket urobil
    fprintf(o,"socket uspesny na adrese %s a porte %i!\n",(char) server_address.sin_addr.s_addr,port_udp);
  }
//pomenovanie socketu
  if(bind(UDP_socket,(struct sockaddr*) &server_address,sizeof(server_address)) == -1)
  {
    fprintf(e,"neviem pomenovat socket!\n");
    exit(EXIT_FAILURE);
  }
//vyhradenie pamate pre slovo
  slovo = (char *) malloc(sizeof(char)*151);
  if(slovo == NULL)
  {
    fprintf(e,"malo pamate pre slovo!\n");
    exit(EXIT_FAILURE);
  }

//parameter pocitadlo nam hovori ze kolko slov poslali P1 a P2 takze tolko krat zbehne cyklus
  while(pocitadlo != 0)
  {
//cakaj kym nedostanes nejake slovo
    connection = recv(UDP_socket,slovo,150,0);
//zapis ho do suboru
    write(subor_fd,slovo,strlen(slovo));
//nebolo ukoncene znakom noveho riadka takze ho tam pridame
    write(subor_fd,"\n",1);
//kvoli debugingu si to vypiseme
    fprintf(o,"serv2: %s\n",slovo);
//stvrt sekundy si pospi
    usleep(250);
//zniz pocitadlo
    pocitadlo--;
  }

  fprintf(o,"Koncim!\n");
  close(subor_fd);
  close(UDP_socket);
  fclose(o);
  fclose(e);
  exit(EXIT_SUCCESS);
}

//definicia fcie
void termination(int sig)
{
  fclose(o);
  fclose(e);
  close(subor_fd);
  close(UDP_socket);
  (void) signal(SIGINT,SIG_DFL);
  exit(EXIT_SUCCESS);
}
