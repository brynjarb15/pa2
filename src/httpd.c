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
    

    if (argc < 2) {
        // TODO: ERROR stuff
        printf("The port must be a argument \n");
	return -1;
    }
   
    printf("start of the program \n");
    int sockfd;
    struct sockaddr_in server, client;
    char message[512];	
    // Getting the port number frome parameter
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
	char *ipNumberFromClient = inet_ntoa(client.sin_addr);
	int portNumberFromClient = ntohs(client.sin_port);
	//printf("ipNumber: %s:%d\n", ipNumberFromClient, portNumberFromClient);
	ssize_t n = recv(connfd, message, sizeof(message) - 1, 0);
	message[n] = '\0';
        fprintf(stdout, "Received:\n%s\n", message);
	
	// Split up the string at " \r\n"
	char** messageSplit = g_strsplit_set(message, " \r\n", 0); // if last >1 everything is split
	gchar* requestMethod = messageSplit[0];
	char* urlRest = messageSplit[1];
	char* header = "HTTP/1.1 200 OK\n"
		       "Content-Type: text/html\n"
		       "\n";
	char* startOfHtml = "<!doctype html><body><p>";
	char* endOfHtml = "</p></body></html>\n";
	char* startOfUrl = "http://";
	gchar* wholeHtmlCode;
	char portNumber[20];
	sprintf(portNumber, "%d", portNumberFromClient);
	char* connectionHeaderValue = NULL;
	char* hostHeaderValue = NULL;
	char* next = "init";
	for(int i = 0; next!= NULL; i++) {
		if (g_strcmp0(next, "Connection:") == 0) {
			connectionHeaderValue = messageSplit[i+1];
		}
		if (g_strcmp0(next, "Host:") == 0) {
			hostHeaderValue = messageSplit[i+1];
		}
		//printf("%d: %s\n", i, messageSplit[i]);
		next = messageSplit[i+1];
	}
	if (connectionHeaderValue == NULL) {
		printf("Connection header was not found, error stuff");
	}
	if (hostHeaderValue == NULL) {
                printf("Host header was not found, error stuff");
        }
	if(g_strcmp0(requestMethod,"GET") == 0)
	{
	    wholeHtmlCode = g_strconcat(header, startOfHtml, startOfUrl, hostHeaderValue, urlRest ," ",  
					ipNumberFromClient, ":", portNumber, endOfHtml, NULL);
	}
	else if(g_strcmp0(requestMethod,"HEAD") == 0)
	{
	    wholeHtmlCode = g_strconcat(header,NULL);
	}
	else if(g_strcmp0(requestMethod, "POST") == 0)
	{
	    char** split = g_strsplit(message, "\r", -1);
	    char* next = "init";
	    char* body = NULL;
	    
	    for(int i = -1; next != NULL; i++)
	    {
		if(g_strcmp0(next, "\n") == 0 && split[i+1] != NULL)
		{
		   body = split[i+1];
		}
		next = split[i+1];
	    }
	    wholeHtmlCode = g_strconcat(header, startOfHtml, startOfUrl, hostHeaderValue, urlRest, " ", ipNumberFromClient, ":", portNumber, body, endOfHtml, NULL);
	    printf("only the body: %s", body);
	    g_strfreev(split);
	}
	else
	{
	    printf("error! A right request method was not given");
	    exit(1);
	}
	send(connfd, wholeHtmlCode, strlen(wholeHtmlCode), 0);
	g_free(wholeHtmlCode);
	connectionHeaderValue = "close"; // Have to close for now implament the other stuff later
	//int persistent = 0;
	if(g_strcmp0(connectionHeaderValue, "close") == 0 /*|| inactivity í 30sek || (g_strcmp0("HTTP/1.0") == 0 && ekki keep-alive)*/ )
	{	
	    printf("disconnecting");
	    shutdown(connfd, SHUT_RDWR);
    	    close(connfd);
	    //exit(1);
	    //
	    //
	    //á að vera break?
	    //break;
	}	
    }
    // Close the connection
    shutdown(connfd, SHUT_RDWR);
    close(connfd);
    
    printf("end of file\n");
    return 0;
}
