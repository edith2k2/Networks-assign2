#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <netdb.h>

#define BUF_SIZE 100
int main(){
// creating socket
  int sockfd;
  if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0){
    perror("Error in socket creation\n");
    exit(0);
  }
// preparing for bind
  struct sockaddr_in serv_addr;
  serv_addr.sin_port = htons(8181);
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_addr.s_addr = INADDR_ANY;
  socklen_t serv_len = sizeof(struct sockaddr);

// binding
  if(bind(sockfd, (struct sockaddr*)&serv_addr, serv_len) < 0){
    perror("Error in binding \n");
    exit(0);
  }
// iteratively receive
  while(1){
// prepare for receiving
    int recvbytes;
    char buffer[BUF_SIZE];
    struct sockaddr_in cli_addr;
    socklen_t cli_len = sizeof(cli_addr);
// receiving
    if ((recvbytes = recvfrom(sockfd, buffer, BUF_SIZE, 0, (struct sockaddr*)&cli_addr, &cli_len)) < 0){
      perror("error in receive\n");
      exit(0);
    }
    printf("New connection from %s on socket %d\n", inet_ntoa(cli_addr.sin_addr), sockfd);
    printf("Received from client :%s\n", buffer);
// preparing to send to client
    struct hostent *dns = gethostbyname(buffer);
    struct in_addr **addr_list = (struct in_addr **)dns->h_addr_list;
    int buf_len;
// sending to client
    for(int i = 0; addr_list[i] != NULL; i++) {
      memset(buffer, '\0', sizeof(buffer));
      strcpy(buffer, inet_ntoa(*addr_list[i]));
      buf_len = strlen(buffer) + 1;
      sendto(sockfd, buffer, buf_len, 0, (const struct sockaddr*)&cli_addr, cli_len);
      // printf("%s \n", inet_ntoa(*addr_list[i]));
    }
    memset(buffer, '\0', strlen(buffer));
    strcpy(buffer, "exit");
    buf_len = strlen(buffer);
    sendto(sockfd, buffer, buf_len, 0, (const struct sockaddr*)&cli_addr, cli_len);
  }

}
