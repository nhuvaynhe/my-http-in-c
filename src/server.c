#include <time.h>
#include "socket.h"
#include "file.h"
#include "mime.h"

#define PORT         "3490"
#define SERVER_FILES "./serverfiles"

#ifdef DEBUG
#else
    #define printf(...)
#endif 

char* http_get_current_time()
{
    time_t rawtime;
    struct tm *timeinfo;

    time (&rawtime);
    timeinfo = localtime(&rawtime);
    printf("time: Current local time and date: %s", asctime(timeinfo));

    return asctime(timeinfo);
}

static int http_send_response(int sockfd, char *header, 
                              char *content_type, void *body, 
                              int content_length)
{ 
    const int max_response_size = 262144;
    char response[max_response_size];
    char *ptr_resp = response;
    int resplen;
    char* timestr = http_get_current_time();
  
    /* Here we just go after each line to create response buffer */
    resplen = snprintf(ptr_resp, 
                       max_response_size, 
                       "%s\n%s", header, timestr);

    resplen += snprintf(ptr_resp + resplen,
                        max_response_size - resplen, 
                        "\nConnection: close\n");

    resplen += snprintf(ptr_resp + resplen, 
                        max_response_size - resplen, 
                        "Content-Length: %d\n", content_length);

    resplen += snprintf(ptr_resp + resplen, 
                        max_response_size - resplen, 
                        "Content-Type: %s\n", content_type);

    resplen += snprintf(ptr_resp + resplen, 
                        max_response_size - resplen, 
                        "\n%s\n", (char *) body);

    printf("recv: response len is %d\n", resplen);

    int rv = send(sockfd, response, resplen, 0);
    if (rv < 0) {
        perror("send");
    }

    return rv;
}

/* 
 * send a 404 response 
 */
static void resp_404(int fd)
{
    char filepath[4096];
    struct file_data *filedata;
    char *mime_type;

    // Fetch the 404.html file
    snprintf(filepath, sizeof filepath, "%s/404.html", SERVER_FILES);
    filedata = file_load(filepath);

    if (filedata == NULL) {
        fprintf(stderr, "cannot file system 404 file.\n");
        exit(3);
    }

    mime_type = mime_type_get(filepath);

    http_send_response(fd, "HTTP/1.1 404 NOT FOUND", mime_type,
                    filedata->data, filedata->size);

    file_free(filedata);
}

/* 
 * HTTP utils
 */
int is_the_same(char *str1, char *str2) {
    if (strcmp(str1, str2) == 0) {
        return 1;
    }
    return 0;
}

/* 
 * POST and GET handler
 */
int http_GET_handler() {
    printf("get: handle GET request.\n");
    return 0;
}

int http_POST_request() {
    printf("post: handle POST request.\n");
    return 0;
}

/* 
 * Handle HTTP request and send response
 */
void http_handle_request(int fd)
{
    const int request_buffer_size = 65536; // 64K
    char request[request_buffer_size];

    // Read request
    int bytes_recvd = recv(fd, request, request_buffer_size - 1, 0);
    if (bytes_recvd < 0) {
        perror("recv");
        return;
    }

    printf("recv: we received %d bytes from %d.\n", bytes_recvd, fd);
    printf("recv: the request is:\n");
    printf("\n%s", request);

    /* 
     * Read the first two components of the first line of the request.
     *
     * If GET, handle the get endpoints.
     *      Check if it's /d20 and handle that special case.
     *      Otherwise serve the requested file by calling get_file()
     *
     * If POST, handle the POST request.
     */

    char type_of_request[5];
    char http_version[20];

    sscanf(request, "%s %*s %s\r\n", type_of_request, http_version);
    printf("recv: %s request | http %s.\n", 
            type_of_request, http_version);

    if (is_the_same("GET", type_of_request)) {
        http_GET_handler();
    }
    else if (is_the_same("POST", type_of_request)) {
        http_POST_request();
    }
    else {
        perror("recv: Invalid request");
        exit(1);
    }
}

int main()
{
    int new_fd;
    struct sockaddr_storage their_addr;
    char s[INET6_ADDRSTRLEN];

    int listenfd = socket_get_listener(PORT);
    if (listenfd < 0) {
        fprintf(stderr, "webserver: fatal error getting listening socket\n");
        exit(1);
    }

    /* This is the main loop that accepts incoming connections and
     * response to the request. The main parent process then goes 
     * back to waiting for new connections. */
    printf("webserver: waiting for connections...\n");

    while(1) {
        socklen_t sin_size = sizeof(their_addr);

        new_fd = accept(listenfd, (struct sockaddr *)&their_addr, &sin_size);
        if (new_fd == -1) {
            perror("accept");
            continue;
        }

        inet_ntop(their_addr.ss_family, 
                socket_get_in_addr((struct sockaddr *)&their_addr), 
                s, sizeof(s));
        printf("server: got connection from %s\n", s);

        http_handle_request(new_fd);

        close(new_fd);
    }

    return 0;
}
