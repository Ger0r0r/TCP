#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
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
	char inside_buf[BUFSIZE];
	char outside_buf[BUFSIZE];

	int optval;
	int n;

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

	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);

	server_addr.sin_port = htons((unsigned short)portno);

	if (bind(server_parent_fd, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0)  
		error("on binding");

	if (listen(server_parent_fd, 5) < 0)  
		error("on listen");

	client_len = sizeof(client_addr);
	while (1) {

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

		bzero(outside_buf, BUFSIZE);
		n = read(server_child_fd, outside_buf, BUFSIZE);
		if (n < 0)  
			error("reading from socket");
		printf("server received %d bytes: %s", n, outside_buf);

		n = write(server_child_fd, outside_buf, strlen(outside_buf));
		if (n < 0)  
			error("writing to socket");
                                  
		close(server_child_fd);
	}		

	return 0;
}
