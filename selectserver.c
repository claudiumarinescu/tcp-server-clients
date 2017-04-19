#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define MAX_CLIENTS	5
#define BUFLEN 256

void error(char *msg)
{
	perror(msg);
	exit(1);
}

int main(int argc, char *argv[])
{
	int sockfd, newsockfd, portno, clilen;
	char buffer[BUFLEN], aux[BUFLEN], aux2[BUFLEN];
	struct sockaddr_in serv_addr, cli_addr;
	int n, i, j, check;
	int clients[5];
	int nrClients = 0;

	fd_set read_fds;	//multimea de citire folosita in select()
	fd_set tmp_fds;	//multime folosita temporar 
	int fdmax;		//valoare maxima file descriptor din multimea read_fds

	if (argc < 2) {
		fprintf(stderr,"Usage : %s port\n", argv[0]);
		exit(1);
	}

	// golim multimea de descriptori de citire (read_fds) si multimea tmp_fds 
	FD_ZERO(&read_fds);
	FD_ZERO(&tmp_fds);
	 
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0) 
		error("ERROR opening socket");
	 
	portno = atoi(argv[1]);

	memset((char *) &serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;	// foloseste adresa IP a masinii
	serv_addr.sin_port = htons(portno);
	
	if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(struct sockaddr)) < 0) 
		error("ERROR on binding");
	
	listen(sockfd, MAX_CLIENTS);

	// adaugam noul file descriptor (socketul pe care se asculta conexiuni) 
	// in multimea read_fds
	FD_SET(sockfd, &read_fds);
	fdmax = sockfd;

	// main loop
	while (1) {
		tmp_fds = read_fds; 
		if (select(fdmax + 1, &tmp_fds, NULL, NULL, NULL) == -1) 
			error("ERROR in select");
	
		for(i = 0; i <= fdmax; i++) {
			if (FD_ISSET(i, &tmp_fds)) {
			
				if (i == sockfd) {
					// a venit ceva pe socketul inactiv(cel cu listen) = o noua conexiune
					// actiunea serverului: accept()
					clilen = sizeof(cli_addr);
					if ((newsockfd = accept(sockfd, (struct sockaddr *)&cli_addr, &clilen)) == -1) {
						error("ERROR in accept");
					} 
					else {
						// adaug noul socket intors de accept() la multimea descriptorilor de citire
						FD_SET(newsockfd, &read_fds);
						if (newsockfd > fdmax) { 
							fdmax = newsockfd;
						}

						// trimit info cine mai e conectat
						memset(aux, 0, BUFLEN);
						sprintf(aux, "server\n===Welcome!\n===Clients connected:-");
						int pos = strlen(aux);
						clients[nrClients++] = newsockfd;

						if (nrClients == 1) {
							sprintf(aux+pos, "none-");
						}
						else {
							for (int j = 0; j < nrClients - 1; j++) {
								if (clients[j] == 0)
									continue;
								sprintf(aux+pos, "%d-", clients[j]);
								pos += 2;
								
								memset(aux2, 0, BUFLEN);
								sprintf(aux2, "server\n===>>Client %d just got in the mix<<===", clients[nrClients - 1]);
								check = send(clients[j], aux2, BUFLEN, 0);
								if (check == -1) {
									printf("[SERVER]-----Error sending message!\n");
									return 1;
								}
							}
							
						}
						check = send(newsockfd, aux, BUFLEN, 0);
						if (check == -1) {
							printf("[SERVER]-----Error sending message!\n");
							return 1;
						}
						printf("Mesaj trimis catre clientul de pe socketul %d\n", newsockfd);

					}
					printf("Noua conexiune de la %s, port %d, socket_client %d\n ", 
					inet_ntoa(cli_addr.sin_addr), ntohs(cli_addr.sin_port), newsockfd);
				}
					
				else {
					// am primit date pe unul din socketii cu care vorbesc cu clientii
					// actiunea serverului: recv()
					memset(buffer, 0, BUFLEN);
					if ((n = recv(i, buffer, sizeof(buffer), 0)) <= 0) {
						if (n == 0) {
							//conexiunea s-a inchis; stergere client i din lista
							printf("selectserver: socket %d hung up\n", i);
							for (int j = 0; j < nrClients; j++) {
								if (clients[j] == i) {
									while(j != nrClients) {
										clients[j] = clients[++j];
									}
									nrClients--;
									break;
								}
							}

							// trimitere actualizari cu privire la clientii conectati
							for (int j = 0; j < nrClients; j++) {
								memset(aux, 0, BUFLEN);
								sprintf(aux, "server\n===>>Client %d just left<<===\n", i);
								int pos = strlen(aux);

								check = send(clients[j], aux, BUFLEN, 0);
								if (check == -1) {
									printf("[SERVER]-----Error sending message!\n");
									return 1;
								}
							}

						} else {
							error("ERROR in recv");
						}
						close(i); 
						FD_CLR(i, &read_fds); // scoatem din multimea de citire socketul pe care 
					} 
					
					else { //recv intoarce >0
						printf ("Am primit de la clientul de pe socketul %d, mesajul: %s\n", i, buffer);

						int dest = atoi(buffer);
						memset(aux, 0, BUFLEN);

						if (FD_ISSET(dest, &read_fds)) {
							sprintf(aux, "%d:", i);
							sprintf(aux+2, "%s", buffer+1);
							check = send(dest, aux, BUFLEN, 0);
							if (check == -1) {
								printf("[SERVER]-----Error sending message!\n");
								return 1;
							}
							printf("Mesaj trimis catre clientul de pe socketul %d\n", dest);
						}
						else {
							sprintf(aux, "->%d<- nu este conectat.", dest);
							check = send(i, aux, BUFLEN, 0);
							if (check == -1) {
								printf("[SERVER]-----Error sending message!\n");
								return 1;
							}
							printf("Mesaj trimis catre clientul de pe socketul %d\n", dest);
						}
					}
				} 
			}
		}
	 }


	 close(sockfd);
   
	 return 0; 
}


