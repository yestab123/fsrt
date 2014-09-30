#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <sys/types.h>
#include <stdint.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <math.h>
#include <netdb.h>
#include <time.h>
#include <stdint.h>

#include "tool.h"


uint32_t 
get_now_sec(void) {
    
    time_t sec;
    struct timespec spec;

    clock_gettime(CLOCK_REALTIME, &spec);

    sec = spec.tv_sec;

    return sec;
}

uint64_t
get_now_msec(void) {
    
    uint64_t ms;
    time_t sec;
    struct timespec spec;

    clock_gettime(CLOCK_REALTIME, &spec);

    sec = spec.tv_sec;
    ms = round(spec.tv_nsec / 1.0e6);

    return sec * 1.0e3 + ms;
}

int
set_reuseaddr(int fd) {
    
    int reuse;
    socklen_t len;

    reuse = 1;
    len = sizeof(reuse);

    return setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &reuse, len);
}

uint16_t  
bytes_to_ui16(const char *s, int from_big_endian) {
    
    unsigned char *p = (void *)s;
    if (from_big_endian) {
        
        return 256U*p[0] + p[1];
    } else {
        
        return p[0] + 256U*p[1];
    }
}

uint32_t
bytes_to_ui32(const char *s, int from_big_endian) {
    
    unsigned char *p = (void *)s;
    if (from_big_endian) {
        
        return 16777216U*p[0] + 65536U*p[1] + 256U*p[2] + p[3];
    } else {
        
        return p[0] + 256U*p[1] + 65536U*p[2] + 16777216U*p[3];
    }
}

void
ui16_to_bytes(char *s, uint16_t n) {
    
    unsigned char *p = (void *)&n;
    int i;

    for (i = 0; i < 2; i++) {
        
        s[i] = p[1 - i];
    }
}

void
ui32_to_bytes(char *s, uint32_t n) {
    
    unsigned char *p = (void *)&n;
    int i;

    for (i = 0; i < 4; i++) {
        
        s[i] = p[3 - i];
    }
}

void
ui8_to_bytes(char *s, uint8_t n) {
    
    unsigned char *p = (void *)&n;

    s[0] = p[0];
}

uint8_t
bytes_to_ui8(const char *s) {
    
    unsigned char *p = (void *)s;

    return p[0];
}

char *
get_file_name(char *file_name) {
    char *p;

    p = strrchr(file_name, '/');
    if (p == NULL) {
        return file_name;
    } 
    return (p + 1);
}

int
init_proto(char *file_name, char *proto, int file_size, int *llen) {
    uint16_t flag;
    uint32_t len;

    flag = COMPARE_FLAG;
    len = strlen(file_name) + PROTO_LEN + PROTO_FLAG + PROTO_FILE;
    ui32_to_bytes(proto, len);
    ui16_to_bytes(proto + PROTO_LEN, flag);
    ui32_to_bytes(proto + PROTO_LEN + PROTO_FLAG, file_size);
    strncpy(proto + PROTO_LEN + PROTO_FLAG + PROTO_FILE, file_name, 
            strlen(file_name));

    *llen = len; 
    return 0;
}

int 
parse_proto(char *file_name, char *proto, int *file_size) {
    uint32_t len;
    uint16_t flag;
    int name_len;

    flag = bytes_to_ui16(proto + PROTO_LEN, 1);
    if (flag != COMPARE_FLAG) {
        printf("get wrong flag %X\n", flag);
        exit(0);
    }
    
    len = bytes_to_ui32(proto, 1);
    *file_size = bytes_to_ui32(proto + PROTO_LEN + PROTO_FLAG, 1);
    name_len = len - PROTO_LEN - PROTO_FLAG - PROTO_FILE;

    strncpy(file_name, proto + PROTO_LEN + PROTO_FLAG + PROTO_FILE,
            name_len);
    return 0;
}
