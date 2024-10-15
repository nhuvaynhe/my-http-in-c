#include "socket.h"

/*
 * This gets and Internet address, either IPv4 or IPv6.
 */
void *socket_get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &( ((struct sockaddr_in*)sa)->sin_addr );
    }

    return &( ((struct sockaddr_in6*)sa)->sin6_addr );
}

/*
 * Return the main listening socket
 */
int socket_get_listener(char *port)
{
    int sockfd, new_fd;
    struct addrinfo hints, *servinfo, *p;
    int rv;
    int yes=1;

    /* This block of code looks at the local network interfaces and
     * tries to find some that match our requirements, namely either
     * IPv4 or IPv6 (AF_UNSPEC) and TCP (SOCK_STREAM) and use any IP
     * on this machine (AI_PASSIVE). */

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    rv = getaddrinfo(NULL, port, &hints, &servinfo);
    if (rv != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }

    // loop through all the results and bind to the first we can
    for(p = servinfo; p != NULL; p = p->ai_next) {

        /* try to make a socket based on this candidate interface */
        sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (sockfd == -1) {
            perror("server: socket");
            continue;
        }

        /* SO_REUSEADDR prevents the "address already in use" errors,
         * that commonly come up when testing servers. */
        if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, 
                    &yes, sizeof(int)) == -1) {
            perror("setsockopt");
            exit(1);
        }

        if(bind(sockfd, p->ai_addr, p->ai_addrlen)) {
            close(sockfd);
            perror("server: bind");
            continue;
        }

        break;
    }

    freeaddrinfo(servinfo); // all done with this structure

    /* If p is NULL, it means that we don't have a good socket. */
    if (p == NULL) {
        fprintf(stderr, "server: failed to bind\n");
        exit(1);
    }

    /* Start listening to sockfd */
    /* This is what allows remote computers to connect to this socket/IP */
    if (listen(sockfd, BACKLOG) == -1) {
        fprintf(stderr, "listen");
        exit(1);
    }

    return sockfd;
}


