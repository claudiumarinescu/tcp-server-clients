#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 
#include <arpa/inet.h>

#define BUFLEN 256

void error(char *msg)
{
	perror(msg);
	exit(0);
}

int main(int argc, char *argv[])
{
	int sockfd, n, i;
	struct sockaddr_in serv_addr;
	struct hostent *server;

	fd_set read_fds;	//multimea de citire folosita in select()
	fd_set tmp_fds; //multime folosita temporar 
	int fdmax;	  //valoare maxima file descriptor din multimea read_fds

	FD_ZERO(&read_fds);
	FD_ZERO(&tmp_fds);

	char buffer[BUFLEN];
	if (argc < 3) {
	   fprintf(stderr,"Usage %s server_address server_port\n", argv[0]);
	   exit(0);
	}
	
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0) 
		error("ERROR opening socket");
	
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(atoi(argv[2]));
	inet_aton(argv[1], &serv_addr.sin_addr);
	
	
	if (connect(sockfd,(struct sockaddr*) &serv_addr,sizeof(serv_addr)) < 0) 
		error("ERROR connecting");	

	FD_SET(0, &read_fds);
	FD_SET(sockfd, &read_fds);
	fdmax = sockfd;

	while(1){

		tmp_fds = read_fds; 
		if (select(fdmax + 1, &tmp_fds, NULL, NULL, NULL) == -1) 
			error("[CLIENT]-----ERROR in select");

		// citire de la tastatura si trimitere mesaj
		memset(buffer, 0, BUFLEN);
		if (FD_ISSET(0, &tmp_fds)) {
			fgets(buffer, BUFLEN-1, stdin);
			buffer[strlen(buffer) - 1] = '\0';
			if (strcmp(buffer, "exit") == 0) {
				close(sockfd);
				return 0;
			}

			int check = send(sockfd, buffer, BUFLEN, 0);
			if (check == -1) {
				printf("[CLIENT]-----Error sending message!\n");
				return 1;
			}
		}

		// citire de pe socket
		if (FD_ISSET(sockfd, &tmp_fds)) {
			if ((n = recv(sockfd, buffer, sizeof(buffer), 0)) <= 0) {
				if (n == 0) {
					//conexiunea s-a inchis
					printf("[CLIENT]-----socket %d hung up\n", i);
				} else {
					error("[CLIENT]-----ERROR in recv");
				}
				close(sockfd); 
				FD_CLR(sockfd, &read_fds); // scoatem din multimea de citire socketul pe care 
			} 
			else { //recv intoarce >0
				printf ("[CLIENT]-----Received from %s\n", buffer);
			}
		}
	}



	return 0;
}


