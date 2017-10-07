#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <ctype.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>

int main(int argc, char *argv[]) {
    
    char* htmlCode = "HTTP/1.1 200 OK\n"
    "Date: Mon, 27 Jul 2009 12:28:53 GMT\n"
    "Server: Apache\n"
    "Last-Modified: Wed, 22 Jul 2009 19:15:56 GMT\n"
    "ETag: \"34aa387-d-1568eb00\"\n"
    "Accept-Ranges: bytes\n"
    "Content-Length: 51\n"
    "Vary: Accept-Encoding\n"
    "Content-Type: text/html\n"
    "\n"
    //"Hello World! My payload includes a trailing CRLF.\n";
    "<!doctype html><body><h1>Page</h1></body></html>";
    printf("start of the program \n");
    int sockfd;
    struct sockaddr_in server, client;
    char message[512];	
    // Getting 
    int port = strtol(argv[1], NULL, 10);
	
    // Create and bind a TCP socket.
    sockfd = socket(AF_INET, SOCK_STREAM, 0);

    // Network functions need arguments in network byte order instead of
    // host byte order. The macros htonl, htons convert the values.
    memset(&server, 0, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = htonl(INADDR_ANY);
    server.sin_port = htons(port);
    bind(sockfd, (struct sockaddr *) &server, (socklen_t) sizeof(server));
	// Before the server can accept messages, it has to listen to the
    // welcome port. A backlog of one connection is allowed.
    listen(sockfd, 1);

    int connfd;
    for (;;) {
	printf("begging of for loop\n");
    	// We first have to accept a TCP connection, connfd is a fresh
        // handle dedicated to this connection.
        socklen_t len = (socklen_t) sizeof(client);
        connfd = accept(sockfd, (struct sockaddr *) &client, &len);
	char *ipNumber = inet_ntoa(client.sin_addr);
	int portNumberFromClient = ntohs(client.sin_port);
	printf("ipNumber: %s:%d\n", ipNumber, portNumberFromClient);
	ssize_t n = recv(connfd, message, sizeof(message) - 1, 0);
	message[n] = '\0';
        fprintf(stdout, "Received:\n%s\n", message);
	for (int i = 0; i < n; ++i) message[i] = toupper(message[i]);
	size_t sizeOfHtml = sizeof(htmlCode);
	send(connfd, htmlCode, strlen(htmlCode), 0);
	// Close the connection.
    //	shutdown(connfd, SHUT_RDWR);
       // close(connfd);
	printf("end of for loop\n");
    }
    // Close the connection
    shutdown(connfd, SHUT_RDWR);
    close(connfd);
    
    printf("end of file\n");
    return 0;
}
