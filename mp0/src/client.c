/*
 ** client.c -- a stream socket client demo
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>

#include <arpa/inet.h>

#define PORT "80" // http only accepts ports 80

#define MAXDATASIZE 100 // max number of bytes we can get at once
// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int main(int argc, char *argv[])
{
    int sockfd, numbytes;
    char buf[MAXDATASIZE];
    struct addrinfo hints, *servinfo, *p;
    int rv;
    char s[INET6_ADDRSTRLEN];

    if (argc != 2) {
        fprintf(stderr,"usage: client hostname\n");
        exit(1);
    }



    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    //parse the command line URL
    char hostname[50];
    int port[4];
    char identifier[50];
    int pro = 0;
    int i,j,u;
    int k =0;
    //printf("%c\n",argv[1][6]);
    for(i = 7; i < strlen(argv[1]);i++){
      if(argv[1][i] == ':'){
        pro = -2;
        for(j =0; j < i;j++){
          hostname[j] = argv[1][j];
          printf("host:");
          printf("%c\n",hostname[j]);
         }
      }
      if(pro == -2){
        for(u = 0; u<4;u++){
          port[u] = argv[1][i+u+1] - '0';
          printf("port");
          printf("%d\n",port[u]);
        }
        pro =-1;
      }
      if(pro == -1 && i+5 <strlen(argv[1])){

        identifier[k] = argv[1][i+5];
        printf("identifier");
        printf("%c\n",identifier[k]);
        k++;

      }

      if(argv[1][i] == '/' && pro == 0){
        port[0] = '8' - '0';
        port[1] = '0' - '0';
        port[2] = NULL;
        port[3] = NULL;
        printf("port");
        printf("%c\n",port[0]);
        printf("port");
        printf("%c\n",port[1]);
        pro = 1;
        for(j =0; j < i;j++){
          hostname[j] = argv[1][j];
          printf("host/");
          printf("%c\n",hostname[j]);
        }
      }
      if(pro == 1){
        identifier[k] = argv[1][i];
        printf("identifier");
        printf("%c\n",identifier[k]);
        k++;
      }

      }

      // format the information about the HTTP GET that wget uses
      char wget_buf[100];
      char wget_buf_tail[100];
      strcat(wget_buf,"GET ");
      strcat(wget_buf,identifier);
      strcat(wget_buf," HTTP/1.1\n");
      strcat(wget_buf,"User-Agent:  Wget/1.12 (linux-gnu)\n");
      strcat(wget_buf,"Host: localhost: ");
      // condition on port number 2digit or 4 digit 
      printf("%s",wget_buf);
      printf("%d",port[0]);
      printf("%d",port[1]);
      printf("%d",port[2]);
      printf("%d\n",port[3]);
      strcat(wget_buf_tail,"Connection: Keep-Alive\n");
      printf("%s\n",wget_buf_tail);
    
//
    
    if ((rv = getaddrinfo(argv[1], PORT, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }

    // loop through all the results and connect to the first we can
    for(p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype,
                             p->ai_protocol)) == -1) {
            perror("client: socket");
            continue;
        }

        if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            perror("client: connect");
            continue;
        }

        break;
    }

    if (p == NULL) {
        fprintf(stderr, "client: failed to connect\n");
        return 2;
    }
//
    inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr),
              s, sizeof s);
    printf("client: connecting to %s\n", s);

    freeaddrinfo(servinfo); // all done with this structure

    if ((numbytes = recv(sockfd, buf, MAXDATASIZE-1, 0)) == -1) {
        perror("recv");
        exit(1);
    }

    buf[numbytes] = '\0';

    printf("client: received\n%s\n",buf);
    printf("client: received %d bytes\n",numbytes);


    close(sockfd);

    return 0;
}
