#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define BUFSIZE 65536

void error(char *msg) {
	char er_hello[10] = "ERROR ";
	char * er_mes = calloc(strlen(msg) + strlen(er_hello), sizeof(char));
	er_mes = strcat(er_hello, msg);
	perror(er_mes);
	free(er_mes);
	exit(1);
}

int main(int argc, char **argv) {
	int server_parent_fd, server_child_fd;
	int parent_fd, child_fd;
	int portno;
	struct sockaddr_in server_addr, client_addr;
	struct hostent * hostp;

	char * host_addrp;

	int client_len;
	char buf_to[BUFSIZE];
	char buf_from[BUFSIZE];

	int optval;
	int n;

	int check = 1;

	if (argc !=2) {
		fprintf(stderr, "usage: %s <port>\n", argv[0]);
		exit(1);
	}
	portno = atoi(argv[1]);

	server_parent_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (parent_fd < 0) 
		error("opening socket");

	optval = 1;
	setsockopt(server_parent_fd, SOL_SOCKET, SO_REUSEADDR, (const void *)&optval , sizeof(int));

	bzero((char *) &server_addr, sizeof(server_addr));

	server_addr.sin_family = AF_INET;

	server_addr.sin_addr.s_addr = INADDR_ANY;

	server_addr.sin_port = htons((unsigned short)portno);

	if (bind(server_parent_fd, (struct sockaddr *) &server_addr, sizeof(server_addr)) != 0)  
		error("on binding");

	printf("Server IP - %s:%u\n", inet_ntoa(server_addr.sin_addr), ntohs(server_addr.sin_port));

	if (listen(server_parent_fd, 5) < 0)  
		error("on listen");

	client_len = sizeof(client_addr);

	server_child_fd = accept(server_parent_fd, (struct sockaddr *) &client_addr, (socklen_t *)&client_len);
	if (server_child_fd < 0)  
		error("on accept");

	hostp = gethostbyaddr((const char *)&client_addr.sin_addr.s_addr, sizeof(client_addr.sin_addr.s_addr), AF_INET);
	if (hostp == NULL)
		error("on gethostbyaddr");
	host_addrp = inet_ntoa(client_addr.sin_addr);
	if (host_addrp == NULL)
		error("on inet_ntoa\n");
	printf("server established connection with %s (%s)\n", hostp->h_name, host_addrp);

	child_fd = fork();
	
	if (child_fd > 0) { // PARENT
		while (1) {
			bzero(buf_to, BUFSIZE);

			n = read(STDIN_FILENO, buf_to, BUFSIZE);
			if (n < 0)
				error("reading from stdin");

			if (strstr(buf_to, "KILL")) {
				check = 0;
				kill(server_child_fd, 2);
				break;
			} else check = 1;

			n = write(server_child_fd, buf_to, strlen(buf_to));
			if (n < 0)
				error("writing to socket"); 
		}
	} else if (server_child_fd == 0) {	// CHILD
		while (1) {
			bzero(buf_from, BUFSIZE);

			n = read(server_child_fd, buf_from, BUFSIZE);
			if (n < 0)
				error("reading from socket");

			if (strstr(buf_from, "KILL")) {
				check = 0;
				kill(getpid(), 2);
				break;
			} else check = 1;

			n = write(STDOUT_FILENO, buf_from, strlen(buf_from));
			if (n < 0)
				error("writing to stdout");
		}
	} else { // ERROR
		error("on fork");
	}

	close(server_child_fd);

	return 0;
}
