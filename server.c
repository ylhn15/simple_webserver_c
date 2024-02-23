#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>

int main(int argc, char* argv[]) {
	int port = 0;
	if(argc != 2) {
		port = 9999;
	} else {
		char* p;
		port = strtol(argv[1], &p, 10);
		if(*p != '\0' || errno != 0) {
			return 1;
		}
	}
	char buff[1024];
	char index_resp[] = "HTTP/1.0 200 OK\r\n"
		"Server: webserver-c\r\n"
		"Content-type: text/html\r\n\r\n"
		"<html>\n <b>index</b>\n <br /> <a href=\"https://www.lehnhausen.dev\">Visit my page</a> </html>\r\n";
	char resp[] = "HTTP/1.0 200 OK\r\n"
		"Server: webserver-c\r\n"
		"Content-type: text/html\r\n\r\n"
		"<html>\n <b>we out here livin' large</b>\n <br /> <a href=\"https://www.lehnhausen.dev\">Visit my page</a> </html>\r\n";
	int tcp_socket = socket(AF_INET, SOCK_STREAM, 0);
	if(tcp_socket < 0) {
		perror("Error creating socket\n");
		return 1;
	};
	struct sockaddr_in host_addr;
	int host_addrlen = sizeof(host_addr);
	host_addr.sin_family = AF_INET;
	host_addr.sin_port = htons(port);
	host_addr.sin_addr.s_addr = INADDR_ANY;
	struct sockaddr_in client_addr;
	int client_addrlen = sizeof(client_addr);
	int bind_result = bind(tcp_socket, (struct sockaddr *)&host_addr, host_addrlen);
	if(bind_result == -1) {
		if(errno == 98) {
			printf("Error binding address\nAddress %d already in use\n",  ntohs(host_addr.sin_port));
		} else {
			printf("Unknown error with errno %d", errno);
		}
		return 1;
	}
	printf("Start webserver listening on port %d\n", ntohs(host_addr.sin_port));
	int listener =  listen(tcp_socket, SOMAXCONN);
	if(listener < 0) {
		perror("Error listening\n");
		return 1;
	}
	for(;;) {
		int accept_socket = accept(tcp_socket, (struct sockaddr *)&host_addr, (socklen_t *)&host_addrlen);
		if(accept_socket < 0) {
			perror("Error accepting connection\n");
			continue;
		}
		int socket_name = getsockname(accept_socket,(struct sockaddr *)&client_addr, (socklen_t *)&client_addrlen);
		if(socket_name < 0) {
			perror("Error getting socket name\n");
			continue;
		}
		int read_val = read(accept_socket, buff, 1024);
		if(read_val < 0) {
			perror("Error reading input\n");
			continue;
		}
		printf("------------------\n");
		printf("[%s:%u]\n", inet_ntoa(client_addr.sin_addr),
				ntohs(client_addr.sin_port));
		char method[1024];
		char uri[1024];
		char version[1024];
		char index_path[] = "/index";
		sscanf(buff, "%s %s %s", method, uri, version);
		printf("Request headers:\n");
		printf("%s\n%s\n%s\n", method, uri, version);
		printf("------------------\n");
		int write_val = 0;
		if(strcmp(uri, index_path) == 0) {
			write_val = write(accept_socket, index_resp, strlen(index_resp));
		} else {
			write_val = write(accept_socket, resp, strlen(resp));
		}
		if(write_val < 0) {
			perror("Error writing input\n");
			continue;
		}
		close(accept_socket);
	}
	return 0;
}

