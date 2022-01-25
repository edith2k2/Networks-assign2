#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

#define BUF_SIZE 100
#define TCP_SERV_PORT 8181

int main(){
// preparing for creation of socket
  int sockfd;
// creating socket
  if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
    perror("error in creation of socket\n");
    exit(0);

  }
// preparing for connect
  struct sockaddr_in serv_addr;
  memset(&serv_addr, '\0', sizeof(serv_addr));

  serv_addr.sin_port = htons(TCP_SERV_PORT);
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_addr.s_addr = INADDR_ANY;

// prompting for input
  char buffer[BUF_SIZE];
  printf("Enter DNS name : ");
  scanf("%s", buffer);
  printf("%s\n", buffer);

// connect
  if(connect(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0){
    perror("Error in connect\n");
    exit(0);
  }
// sending dns info
  int sentbyes;
  socklen_t serv_len = sizeof(serv_addr);
  int buf_len = strlen(buffer) + 1;
  if (send(sockfd, buffer, buf_len, 0) < 0){
    perror("error in send\n");
    exit(0);
  }
// preparing for select
  struct timeval tv;
  tv.tv_sec = 2;
  tv.tv_usec = 0;

  fd_set readfds;
  FD_ZERO(&readfds);
  FD_SET(sockfd, &readfds);

  select(sockfd + 1, &readfds, NULL, NULL, &tv);

  // receiving from server
  while(1){
    int byterec;
    memset(buffer, '\0', strlen(buffer));
    if (! FD_ISSET(sockfd, &readfds)){
      perror("Server timed out\n");
      exit(0);
    }
    if ((byterec = recv(sockfd, buffer, BUF_SIZE - 1, 0)) < 0){
      perror("receive error");
      exit(0);
    }
    if (! strcmp(buffer, "exit")){
      break;
    }
    printf("%s\n",buffer);
    strcpy(buffer, "received");
    if (send(sockfd, buffer, buf_len, 0) < 0){
      perror("Error in send to tcp server\n");
      exit(0);
    }
  }
  close(sockfd);
}
