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
    char* text = "GET /djammid HTTP/1.1\n";
    gchar** a = g_strsplit(text, " ", 2);
    //printf ("test: |%s| \n", a[1]);
    g_strfreev(a);
    char* b = g_strconcat(text, text, NULL);
    //printf("test2: |%s| \n", b);
    g_free(b);
    char* htmlCode = "HTTP/1.1 200 OK\n"
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
    "<!doctype html><body><h1>Page22issssesesdf</h1></body></html>\n";
    printf("start of the program \n");
    
    int sockfd;
    struct sockaddr_in server, client;
    char message[512];
    if (argc < 2) {
        // TODO: ERROR stuff
        printf("The port must be a argument \n");
	return -1;
    }
    // Getting the port
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
        //fprintf(stdout, "Received:\n%s\n", message);
	// Split up the string at " "
	char** messageSplit = g_strsplit_set(message, " \r\n", 0); // if last >1 everything is split
	// number 0 is the request method
	gchar* requestMethod = messageSplit[0];
	
	//printf(": %s \n", requestMethod)
	char* url = g_strdup(messageSplit[5]);
//	printf("herne: %s\n", messageSplit[2]);
//	printf("herness: %s\n", url);
	char* url2 =  g_strdup(messageSplit[1]);

//	strcpy(url, messageSplit[4]);
	char* p = g_strconcat(url, "", url2, NULL);
//	printf("herness: %s\n", url2);
	printf("%s\n", p);
	//gchar* requestRest = g_strdup(messageSplit[4]);
	//strcat(requestUrl, messageSplit[1]);
	             
	//gchar* output = g_strdup_printf("http://", host, NULL);//":", portNumberFromClient, NULL);
	//gchar* output2 = g_strconcat(output, requestRest, NULL);
	gchar* output = g_strdup_printf("http://%s\n", "jeje");
//	printf("reqRest: %s\n", requestRest);
	//free(url);
	fflush(stdout);
	fprintf(stdout, "\noutputosdfgsdfgsdfgsdfgfsdgsdgsdutput: %s\n\n", output);
	//g_printf("http://\n");
	//g_printf("%s\n", 	host);
	//g_printf("%s\n", requestRest);
	//g_printf(" %s:%d\n", ipNumber, portNumberFromClient);
	//g_printf("\n\n");

	//g_printf("Output: http://%s%s %s:%d \n\n", host, requestRest, ipNumber, portNumberFromClient);
	send(connfd, htmlCode, strlen(htmlCode), 0);
	// Close the connection.
    	shutdown(connfd, SHUT_RDWR);
       // close(connfd);
//        g_free(output2);
	

	printf("here\n");
	g_free(url);
	g_free(output);	

        g_strfreev(messageSplit);
printf("here\n");
	printf("end of for loop\n");
printf("here\n");

    }
    // Close the connection
    shutdown(connfd, SHUT_RDWR);
    close(connfd);
    
    printf("end of file\n");
    return 0;
}
