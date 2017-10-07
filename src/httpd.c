#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <ctype.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <glib.h>
#include <glib/gprintf.h>

int main(int argc, char *argv[]) {
    
    char* htmlCode1 = "HTTP/1.1 200 OK\n"
    //"Date: Mon, 27 Jul 2009 12:28:53 GMT\n"
    //"Server: Apache\n"
    //"Last-Modified: Wed, 22 Jul 2009 19:15:56 GMT\n"
    //"ETag: \"34aa387-d-1568eb00\"\n"
    //"Accept-Ranges: bytes\n"
    //"Content-Length: 51\n"
    //"Vary: Accept-Encoding\n"
    "Content-Type: text/html\n"
    "\n"
    //"Hello World! My payload includes a trailing CRLF.\n";
    "<!doctype html><body><h1>";
 
    char* htmlCode3 = "</h1></body></html>";
   
     printf("start of the program \n");
    int sockfd;
    struct sockaddr_in server, client;
    char message[512];	
    //char firstPartOf
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
	//size_t sizeOfHtml = sizeof(htmlCode);
	char* htmlCode2 = ipNumber;

	char requestMethod[] = "GET"; //request strengur frá brynjari
	gchar* wholeHtmlCode;
	char url[] = "url";
	char IPaddress[] = "IP";
	char portNumber[] = "port";
	char body[] = "body";
	char head[] = "head";
	if(g_strcmp0(requestMethod,"GET") == 0)
	{
	    //wholeHtmlCode = g_strconcat(url, IPaddress, portNumber, NULL);
	    wholeHtmlCode = g_strconcat(htmlCode1,htmlCode2,htmlCode3,NULL);
	}
	else if(g_strcmp0(requestMethod,"POST") == 0)
	{
	    //púsla saman header hér?
	    wholeHtmlCode = head;
	}
	else if(g_strcmp0(requestMethod, "HEAD") == 0)
	{
	    wholeHtmlCode = g_strconcat(url, IPaddress, portNumber, body, NULL);
	}
	else
	{
	    printf("error! A right request method was not given");
	    exit(1);
	}
	send(connfd, wholeHtmlCode, strlen(wholeHtmlCode), 0);
	g_free(wholeHtmlCode);
	char connection[] = "close";
	//int persistent = 0;
	if(g_strcmp0(connection, "close") == 0 /*|| inactivity í 30sek || (g_strcmp0("HTTP/1.0") == 0 && ekki keep-alive)*/ )
	{	
	    printf("disconnecting");
	    shutdown(connfd, SHUT_RDWR);
    	    close(connfd);
	    //exit(1);
	    //
	    //
	    //á að vera break?
	    break;
	}	
    }
    // Close the connection
    shutdown(connfd, SHUT_RDWR);
    close(connfd);
    
    printf("end of file\n");
    return 0;
}
