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
#define TCP_SERV_PORT 8181
#define UDP_SERV_PORT 8181
#define BACKLOG 10

int max(int a, int b){
  if (a > b){
    return a;
  }else{
    return b;
  }
}

int main(){
// preparing for creation of socket
  int tcp_sockfd, udp_sockfd;
  char buffer[BUF_SIZE];
// socket creation
  if ((tcp_sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
    perror("Error in tcp socket creation\n");
    exit(0);
  }
  if ((udp_sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0){
    perror("Error in udp socket creation\n");
    exit(0);
  }
  // preparing for bind
  struct sockaddr_in tcp_serv, udp_serv;

  tcp_serv.sin_port = htons(TCP_SERV_PORT);
  tcp_serv.sin_addr.s_addr = INADDR_ANY;
  tcp_serv.sin_family = AF_INET;

  udp_serv.sin_port = htons(UDP_SERV_PORT);
  udp_serv.sin_addr.s_addr = INADDR_ANY;
  udp_serv.sin_family = AF_INET;

  // losing the "address already in use " message
  int yes = 1;
  if (setsockopt(tcp_sockfd,SOL_SOCKET,SO_REUSEADDR,&yes,sizeof(int)) == -1) {
    perror("setsockopt");
    exit(1);
  }
  if (setsockopt(udp_sockfd,SOL_SOCKET,SO_REUSEADDR,&yes,sizeof(int)) == -1){
    perror("setsockopt");
    exit(1);
  }
  // binding
  if (bind(tcp_sockfd, (struct sockaddr*)&tcp_serv, sizeof(struct sockaddr)) < 0){
    perror("Error in tcp bind\n");
    exit(0);
  }
  if (bind(udp_sockfd, (struct sockaddr*)&udp_serv, sizeof(struct sockaddr)) < 0){
    perror("Error in udp bind\n");
    exit(0);
  }
  // tcp listen
  listen(tcp_sockfd, BACKLOG);

// preparing for select
  fd_set readfds, master;

  FD_ZERO(&master);
  FD_ZERO(&readfds);
  FD_SET(tcp_sockfd, &master);
  FD_SET(udp_sockfd, &master);

// using select
  while(1){
    readfds = master;
    if (select(max(tcp_sockfd, udp_sockfd) + 1, &readfds, NULL, NULL, NULL) == -1){
      perror("error in select\n");
      exit(1);
    }
    // tcp socket
    if (FD_ISSET(tcp_sockfd, &readfds)){
      struct sockaddr_in tcp_cli;
      int new_tcp_sockfd;
      socklen_t sin_size, tcp_cli_len;
      sin_size = sizeof(struct sockaddr_in);
      tcp_cli_len = sizeof(tcp_cli);
      if ((new_tcp_sockfd = accept(tcp_sockfd, (struct sockaddr*)& tcp_cli, &sin_size)) < 0){
        perror("Error in tcp accept\n");
        exit(0);
      }
      pid_t pid;
      pid = fork();
      if (pid < 0){
        perror("error in udp fork\n");
        exit(1);
      }
      if (!pid){// child process
    // receiving dns info from tcp client
        close(tcp_sockfd); // closing child listener
        memset(buffer, '\0', strlen(buffer));
        int tcp_recv;
        if ((tcp_recv = recv(new_tcp_sockfd, buffer, BUF_SIZE - 1, 0)) < 0){
          perror("error in tcp recv\n");
          exit(0);
        }
        printf("new tcp connection from %s on socket %d\n", inet_ntoa(tcp_cli.sin_addr), new_tcp_sockfd);
        printf("received from client : %s\n", buffer);

    // preparing to send to tcp client
        struct hostent *dns = gethostbyname(buffer);
        struct in_addr **addr_list = (struct in_addr **)dns->h_addr_list;
        int buf_len;
    // sending to tcp client
        for(int i = 0; addr_list[i] != NULL; i++) {
          memset(buffer, '\0', sizeof(buffer));
          strcpy(buffer, inet_ntoa(*addr_list[i]));
          buf_len = strlen(buffer) + 1;
          if (send(new_tcp_sockfd, buffer, buf_len, 0) < 0){
            perror("Error in send to tcp client\n");
            exit(0);
          }
          // printf("%s \n", inet_ntoa(*addr_list[i]));
          if (recv(new_tcp_sockfd, buffer, BUF_SIZE - 1, 0) < 0){
            perror("tcp rec error");
            exit(0);
          }
        }
        memset(buffer, '\0', strlen(buffer));
        strcpy(buffer, "exit");
        buf_len = strlen(buffer);
        send(new_tcp_sockfd, buffer, buf_len, 0);
        close(new_tcp_sockfd);
        exit(0);
      }else{
        close(new_tcp_sockfd);// parent only listens
      }
    }
  // udp socket
    if (FD_ISSET(udp_sockfd, &readfds)){
    // prepare for udp receiving
      int recvbytes;
    // char buffer[BUF_SIZE];
      struct sockaddr_in udp_cli_addr;
      socklen_t udp_cli_len = sizeof(udp_cli_addr);
    // udp receiving dns info
      memset(buffer, '\0', strlen(buffer));
      if ((recvbytes = recvfrom(udp_sockfd, buffer, BUF_SIZE, 0, (struct sockaddr*)&udp_cli_addr, &udp_cli_len)) < 0){
        perror("error in receive\n");
        exit(0);
      }
      if (recvbytes > 0){
        pid_t pid = fork();
        if (pid < 0){
          perror("error in udp fork\n");
          exit(1);
        }
        if (!pid){
          printf("new udp connection from %s on socket %d\n", inet_ntoa(udp_cli_addr.sin_addr), udp_sockfd);
          printf("received from client : %s\n", buffer);
        // preparing to send to udp client
          struct hostent *dns = gethostbyname(buffer);
          struct in_addr **addr_list = (struct in_addr **)dns->h_addr_list;
          int buf_len;
        // sending to udp client
          for(int i = 0; addr_list[i] != NULL; i++) {
            memset(buffer, '\0', sizeof(buffer));
            strcpy(buffer, inet_ntoa(*addr_list[i]));
            buf_len = strlen(buffer) + 1;
            sendto(udp_sockfd, buffer, buf_len, 0, (const struct sockaddr*)&udp_cli_addr, udp_cli_len);
            // printf("%s \n", inet_ntoa(*addr_list[i]));
          }
          memset(buffer, '\0', strlen(buffer));
          strcpy(buffer, "exit");
          buf_len = strlen(buffer);
          sendto(udp_sockfd, buffer, buf_len, 0, (const struct sockaddr*)&udp_cli_addr, udp_cli_len);
        }
      }
    }
  }
}
