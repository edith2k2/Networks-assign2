#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

#define BUF_SIZE 100
#define UDP_SERV_PORT 8181
int main(){
// creating socket
  int sockfd;
  if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0){
    perror("Error in creation of socket\n");
    exit(0);
  }
// preparing for send
  struct sockaddr_in serv_addr;
  memset(&serv_addr, 0, sizeof(serv_addr));

  serv_addr.sin_family = AF_INET;
  serv_addr.sin_port = htons(UDP_SERV_PORT);
  serv_addr.sin_addr.s_addr = INADDR_ANY;

// prompting for input
  char buffer[BUF_SIZE];
  printf("Enter DNS name : ");
  scanf("%s", buffer);
  printf("%s\n", buffer);

// sending dns info
  int sentbyes;
  socklen_t serv_len = sizeof(serv_addr);
  int buf_len = strlen(buffer) + 1;
  if ((sentbyes = sendto(sockfd, buffer, buf_len, 0, (const struct sockaddr*)&serv_addr, serv_len)) < 0){
    perror("Error in sendto\n");
    exit(0);
  }
// // binding
  //   if(bind(sockfd, (struct sockaddr*)&cli_addr, cli_len) < 0){
  //     perror("Error in binding \n");
  //     exit(0);
  //   }
// preparing for select
  struct timeval tv;
  tv.tv_sec = 2;
  tv.tv_usec = 0;

  fd_set readfds;
  FD_ZERO(&readfds);
  FD_SET(sockfd, &readfds);

  select(sockfd + 1, &readfds, NULL, NULL, &tv);


//   if (FD_ISSET(sockfd, &readfds)){
// // receiving from server;
//
//   }else{
// // timed out
//
//   }
  for (int i = 0; i < BUF_SIZE; i++){
    buffer[i] = '\0';
  }
  while(1){
    int byterec;
    memset(buffer, '\0', strlen(buffer));
    if (!FD_ISSET(sockfd, &readfds)){
      perror("server timedout\n");
      exit(0);
    }
    if ((byterec = recvfrom(sockfd, buffer, BUF_SIZE, 0, (struct sockaddr*)&serv_addr, &serv_len)) < 0){
      perror("rec error");
      exit(0);
    }
    if (! strcmp(buffer, "exit")){
      break;
    }
    printf("%s\n",buffer);
  }
  close(sockfd);
}
