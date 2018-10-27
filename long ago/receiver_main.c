/*
 * File:   receiver_main.c
 * Author:
 *
 * Created on
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <pthread.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/wait.h>
#include <signal.h>
#include <fcntl.h>


#define BUFSIZE 2000
#define HEADER_SIZE 50
#define DEBUG 0
#define MAXWINDOW 350

struct sockaddr_in si_me, si_other;
int s, slen;

void diep(char *s) {
    perror(s);
    exit(1);
}



void reliablyReceive(unsigned short int myUDPport, char* destinationFile) {

    slen = sizeof (si_other);


    if ((s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
        diep("socket");

    memset((char *) &si_me, 0, sizeof (si_me));
    si_me.sin_family = AF_INET;
    si_me.sin_port = htons(myUDPport);
    si_me.sin_addr.s_addr = htonl(INADDR_ANY);
    printf("Now binding\n");
    if (bind(s, (struct sockaddr*) &si_me, sizeof (si_me)) == -1)
        diep("bind");


    /* Now receive data and send acknowledgements */
    remove(destinationFile);
    //int file_fd = open(destinationFile, O_WRONLY | O_CREAT, 777);
    FILE* fp=fopen(destinationFile,"wb");

    char buf[BUFSIZE];
    struct sockaddr_in sender_addr; //the addr to send back ack
    socklen_t addrlen = sizeof(sender_addr);
    int lastAckSent = -1;

    if (fp ==NULL) {
        fprintf(stderr, "FILE NOT OPENDED\n");
        return;
    }

    int recvbytes = 0;

    while (1) {
    	    //usleep(1000);
        recvbytes = recvfrom(s, buf, BUFSIZE, 0, (struct sockaddr*)&sender_addr, &addrlen);
        if (recvbytes < 0) {
            fprintf(stderr, "Connection closed\n");
            break;
        }

        buf[recvbytes] = '\0';

        if (!strncmp(buf, "EOT", 3)) {
            fprintf(stderr, "end of transmission\n");
            break;
        }
        else {
            char header[HEADER_SIZE];
            int k;
            char* curr;

            for (curr = buf, k = 0; *curr != ';' && k < BUFSIZE; curr++, k++) {
                header[k] = *curr;
            }

            curr++;
            header[k] = '\0';
            int frameIndex;
            sscanf(header, "frame%d", &frameIndex);

            if (frameIndex == lastAckSent + 1) {
                lastAckSent++;
                char ack[HEADER_SIZE];
                sprintf(ack, "ack%d;", frameIndex);

                sendto(s, ack, strlen(ack), 0, (struct sockaddr *) & sender_addr, sizeof(sender_addr));
                fwrite(curr,1,recvbytes-k-1,fp);
                // write(file_fd, curr, recvbytes - k - 1);
                //fsync(file_fd);
            } 
            else if(frameIndex <= lastAckSent){
                char ack[HEADER_SIZE];
                sprintf(ack, "ack%d;", frameIndex); //last ack remained in frameIndex
                sendto(s, ack, strlen(ack), 0, (struct sockaddr *) & sender_addr, sizeof(sender_addr));
            }
			else {
				char ack[HEADER_SIZE];
                sprintf(ack, "ack%d;", lastAckSent); //last ack remained in frameIndex
                sendto(s, ack, strlen(ack), 0, (struct sockaddr *) & sender_addr, sizeof(sender_addr));
			}

            memset(buf, 0, BUFSIZE);//buf reads in a packet every time

        }
    }

    close(s);
    fclose(fp);
    printf("%s received.", destinationFile);
    return;
}

/*
 *
 */
int main(int argc, char** argv) {

    unsigned short int udpPort;

    if (argc != 3) {
        fprintf(stderr, "usage: %s UDP_port filename_to_write\n\n", argv[0]);
        exit(1);
    }

    udpPort = (unsigned short int) atoi(argv[1]);

    reliablyReceive(udpPort, argv[2]);
}

