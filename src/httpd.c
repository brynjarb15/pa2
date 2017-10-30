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
#include <time.h>
#include <sys/time.h>

typedef enum { false,
               true } bool;

void logToFile(char *ipNumber, char *clientPort, char *requestMethod, char *requestedUrl, char *responseCode) {
    // Some ideas of the code below comes from here https://www.tutorialspoint.com/c_standard_library/c_function_strftime.htm
    time_t rawtime;
    struct tm *timeNow;
    char timeStamp[80];
    time(&rawtime);
    timeNow = localtime(&rawtime);
    //ISO 8601 Format: YYYY-MM-DDThh:mm:ssTZD
    strftime(timeStamp, 80, "%Y-%m-%dT%H:%M:%S%Z", timeNow);
    FILE *logFile;
    logFile = fopen("pa2.log", "a");
    if (logFile == NULL) {
        printf("error opening file");
    }
    fprintf(logFile, "%s : %s:%s %s %s : %s \n", timeStamp, ipNumber, clientPort, requestMethod, requestedUrl, responseCode);
    fclose(logFile);
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        // If there is no port the server can't run so we close
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
    bind(sockfd, (struct sockaddr *)&server, (socklen_t)sizeof(server));
    // Before the server can accept messages, it has to listen to the
    // welcome port. A backlog of one connection is allowed.
    listen(sockfd, 1);

    int connfd;
    for (;;) {
        printf("Waiting for connection \n");
        // We first have to accept a TCP connection, connfd is a fresh
        // handle dedicated to this connection.
        socklen_t len = (socklen_t)sizeof(client);
        connfd = accept(sockfd, (struct sockaddr *)&client, &len);
        char *ipNumberFromClient = inet_ntoa(client.sin_addr);
        int portNumberFromClient = ntohs(client.sin_port);
        memset(message, 0, sizeof message);
        // Info about recv from here http://man7.org/linux/man-pages/man2/recv.2.html
        ssize_t n = recv(connfd, message, sizeof(message) - 1, 0);
        message[n] = '\0';
        printf("Connection established\n");
        // Split up the string at " \r\n"
        char **messageSplit = g_strsplit_set(message, " \r\n", 0); // if last >1 everything is split
        gchar *requestMethod = messageSplit[0];                    // e.g. GET
        char *urlRest = messageSplit[1];                           // e.g. /djammid
        char *httpRequestType = messageSplit[2];                   // e.g. HTTP/1.1
        char *statusCode;
        gchar *firstLineOfHeader;
        char *contentTypeHeader = "Content-Type: text/html\r\n";
        char *endOfHeders = "\r\n";
        gchar *header;

        //first and last part of html we send
        char *startOfHtml = "<!doctype html><body><p>";
        char *endOfHtml = "</p></body></html>\r\n";
        char *startOfUrl = "http://";

        gchar *wholeHtmlCode = NULL;
        char portNumber[20];
        sprintf(portNumber, "%d", portNumberFromClient);
        char url[200];
        //the connection and header is gotten from the array that contains the whole message
        char *connectionHeaderValue = NULL;
        char *hostHeaderValue = NULL;
        char *next = "init";
        if (g_strcmp0(requestMethod, "GET") == 0 || g_strcmp0(requestMethod, "HEAD") == 0 ||
            g_strcmp0(requestMethod, "POST") == 0) {
            for (int i = 0; next != NULL; i++) {
                gchar *nextLower = g_ascii_strdown(next, strlen(next));
                if (g_strcmp0(nextLower, "connection:") == 0) {
                    connectionHeaderValue = messageSplit[i + 1];
                }

                if (g_strcmp0(nextLower, "host:") == 0) {
                    hostHeaderValue = messageSplit[i + 1];
                }
                next = messageSplit[i + 1];
                g_free(nextLower);
            }
            if (connectionHeaderValue == NULL) {
                printf("Connection header was not found");
            }
            if (hostHeaderValue == NULL) {
                printf("Host header was not found");
                continue;
            }

            // Make the url out of the the 3 parts
            strcpy(url, startOfUrl);
            strcat(url, hostHeaderValue);
            strcat(url, urlRest);
            //The status code and header of GET POST and HEAD of the request sent back successfully
            statusCode = "200 OK";
            firstLineOfHeader = g_strjoin(" ", httpRequestType, statusCode, "\r\n", NULL);
            header = g_strconcat(firstLineOfHeader, contentTypeHeader, endOfHeders, NULL);

            //Checking what kind of request method to handle
            //
            //In a get request the html page displays the url of the requested page and the IP
            //address and port number of the requesting client
            if (g_strcmp0(requestMethod, "GET") == 0) {
                //favicon is not processed
                if (g_strcmp0(urlRest, "/favicon.ico") == 0) {
                    // Free up memory so we can use them again
                    g_free(firstLineOfHeader);
                    g_free(header);
                    statusCode = "404 Not Found";
                    firstLineOfHeader = g_strjoin(" ", httpRequestType, statusCode, "\r\n", NULL);
                    header = g_strconcat(firstLineOfHeader, contentTypeHeader, endOfHeders, NULL);
                    wholeHtmlCode = g_strconcat(header, "There is no favicon.ico in this server", NULL);
                }
                else {
                    wholeHtmlCode = g_strconcat(header, startOfHtml, url, " ",
                                                ipNumberFromClient, ":", portNumber, endOfHtml, NULL);
                }
            }
            //In a Head request, only the header is returned and nothing is displayed
            else if (g_strcmp0(requestMethod, "HEAD") == 0) {
                wholeHtmlCode = g_strconcat(header, NULL);
            }
            //in a post request the html page displays the url of the requested page, the IP address and port number of the requesting client
            //and the data in the body of the request.
            else if (g_strcmp0(requestMethod, "POST") == 0) {
                char **split = g_strsplit(message, "\r", -1);
                char *next = "init";
                char *body = NULL;
                //all of the data gotten from the body
                for (int i = -1; next != NULL; i++) {
                    if (g_strcmp0(next, "\n") == 0 && split[i + 1] != NULL) {
                        body = split[i + 1];
                    }
                    next = split[i + 1];
                }
                wholeHtmlCode = g_strconcat(header, startOfHtml, startOfUrl, hostHeaderValue, urlRest, " ",
                                            ipNumberFromClient, ":", portNumber, body, endOfHtml, NULL);
                g_strfreev(split);
            }
        }
        else {
            requestMethod = "UNKNOWN";
            statusCode = "501 Not Implemented";
            firstLineOfHeader = g_strjoin(" ", httpRequestType, statusCode, "\r\n", NULL);
            header = g_strconcat(firstLineOfHeader, contentTypeHeader, endOfHeders, NULL);
            wholeHtmlCode = g_strconcat(header, "This service only supports GET, HEAD and POST", NULL);
            // Do this so the connection will be closed after the error message has been sent
            connectionHeaderValue = "close";
        }
        //For each request, a single line is printed to a log file in the format:
        //timestamp: <client ip>:<client port> <request method><requested URL> : <response code>
        logToFile(ipNumberFromClient, portNumber, requestMethod, url, statusCode);

        // We send the wholeHtmlCode constructed above
        send(connfd, wholeHtmlCode, strlen(wholeHtmlCode), 0);

        // Free memory we don't need anymore
        g_free(wholeHtmlCode);
        g_free(firstLineOfHeader);
        g_free(header);
        gchar *headerValueLower = g_ascii_strdown(connectionHeaderValue, strlen(connectionHeaderValue));
        //headerValueLower = "close"; // ------------Þetta á ekki að vera hérna----------------------------
        if (true || g_strcmp0(headerValueLower, "close") == 0 || g_strcmp0("HTTP/1.0", httpRequestType) == 0) {
            g_free(headerValueLower);
            g_strfreev(messageSplit);
            printf("The connection is not persistent so the connection will be closed\n");
            shutdown(connfd, SHUT_RDWR);
            close(connfd);
            continue;
        }
        printf("the connections is persistent so it wont close\n");
        g_strfreev(messageSplit);
        g_free(headerValueLower);
    }
    // Close the connection
    shutdown(connfd, SHUT_RDWR);
    close(connfd);

    printf("Server has been closed\n");
    return 0;
}
