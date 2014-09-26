#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <malloc.h>
#include <error.h>
#include <errno.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdint.h>
#include <math.h>
#include <netdb.h>
#include <time.h>
#include <assert.h>
#include <arpa/inet.h>

#include "connect.h"
#include "define.h"
#include "tool.h"

int 
accept_conn(char *ip, int port) {
    int sockfd;
    int i;
    struct sockaddr_in seraddr;
    
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    set_reuseaddr(sockfd);

    seraddr.sin_addr.s_addr = inet_addr(ip);
    seraddr.sin_port = htons(port);
    seraddr.sin_family = AF_INET;

    i = bind(sockfd, (struct sockaddr*)&seraddr, sizeof(seraddr));
    if (i < 0) {
        printf("bind %s:%d error %s\n", ip, port, strerror(errno));
        exit(0);
    }

    listen(sockfd, 10);

    i = accept(sockfd, NULL, NULL);
    if (i < 0) {
        printf("accept %d error %s\n", sockfd, strerror(errno));
        exit(0);
    }

    return i;
}

int
connect_ser(char *ip, int port) {
    int sockfd;
    int i;
    struct sockaddr_in ser;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    ser.sin_addr.s_addr = inet_addr(ip);
    ser.sin_port = htons(port);
    ser.sin_family = AF_INET;

    i = connect(sockfd, (struct sockaddr*)&ser, sizeof(ser));
    if (i < 0) {
        printf("connect to %s:%d error %s\n", ip, port, strerror(errno));
        exit(0);
    }

    return sockfd;
}

