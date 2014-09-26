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
#include <assert.h>
#include <getopt.h>

#include "tool.h"
#include "define.h"
#include "connect.h"
#include "file.h"

off_t  all_file_size;

int
print_help() {
    printf(
           "Linux File Send/Recv Tool(FSRT)\n"
           "(LizhiFM)\n"
           "Version:%s\n"
           "# You can set this tool to be a client or be a server.\n"
           "# It can be recv or send file when being a client or server.\n"
           "# Now only can support send or recv one file each time.\n"
           "# Recv and Send mode only one can be set.\n"
           "# Client and Server mode only one can be set.\n"
           "\n"
           "# Option:\n"
           "-s/--send                (send)send file mode\n"
           "-r/--recv                (recv)recv file mode\n"
           "-c/--client              (client)client mode\n"
           "-a/--server              (accept)server mode\n"
           "-f/--file [file]         (file)Send file(If -s is set then this must be set)\n"
           "-i/--ip [ip]             (ip)Set conn or listen ip\n"
           "-p/--port [port]         (port)Set conn or listen port\n"
           "-v/-h/--help/--version   (version/help)\n\n",
           VERSION
           );
    return 0;
}

int 
main(int argc, char **argv) {
    uint64_t run;
    float waste;
    float avg;
    char  c;
    char  file[128];
    char  file_name[PROTO_NAME];
    char  ip[32];
    char  command[PROTO_NAME + 100];
    int   port = 0;
    int   file_set = 0;
    int   mode = 0;
    int   connect_mode = 0;
    int   sockfd = 0;

    memset(file_name, '\0', PROTO_NAME);
    memset(ip, '\0', 32);
    memset(file, '\0', 128);
    memset(command, '\0', PROTO_NAME + 100);

    strncpy(ip, DEFAULT_IP, 32);
    port = DEFAULT_PORT;

    struct option long_options[] = {
        { "send", 0, NULL, 's' },
        { "recv", 0, NULL, 'r' },
        { "client", 0, NULL, 'c' },
        { "server", 0, NULL, 'a' },
        { "file", 1, NULL, 'f' },
        { "ip", 1, NULL, 'i' },
        { "port", 1, NULL, 'p' },
        { "help", 0, NULL, 'v' },
        { "version", 0, NULL, 'h' },
        { 0, 0, 0, 0 },
    };
    while((c = getopt_long(argc, argv, "vhsrcai:p:f:", long_options, NULL))!= -1) {
        switch(c) {
        case 's': 
            mode = SEND_MODE;
            break;
        case 'r': 
            mode = RECV_MODE;
            break;
        case 'f': 
            strncpy(file, optarg, 128);
            file_set = 1;
            break;
        case 'c':
            connect_mode = CONN_MODE;
            break;
        case 'a':
            connect_mode = SERV_MODE;
            break;
        case 'i':
            strncpy(ip, optarg, 32);
            break;
        case 'p':
            port = atoi(optarg);
            break;
        case 'v':
        case 'h':
            print_help();
            exit(0);
        }
    }
    
    if (connect_mode == 0) {
        printf("**ERROR**\n");
        printf("**ERROR:connect mode not set\n\n");
        print_help();
        exit(0);
    }

    if (mode == 0) {
        printf("**ERROR**\n");
        printf("**ERROR:send recv mode not set\n\n");
        print_help();
        exit(0);
    }

    if ((mode == SEND_MODE) && (file_set == 0)) {
        printf("**ERROR**\n");
        printf("**ERRORfile not set\n\n");
        print_help();
        exit(0);
    }

    switch(connect_mode) {
    case CONN_MODE:
        sockfd = connect_ser(ip, port);
        break;
    case SERV_MODE:
        sockfd = accept_conn(ip, port);
        break;
    }

    run = get_now_msec();
    switch(mode) {
    case SEND_MODE:
        send_name(sockfd, file);
        send_file(sockfd, file, SEND_BLOCK);
        break;
    case RECV_MODE:
        parse_name(sockfd, file_name);
        recv_file(sockfd, "temp.file");
        break;
    }

    if (mode == RECV_MODE) {
        snprintf(command, PROTO_NAME + 100, "mv temp.file %s", file_name);
        system(command);
    } else {
        run = get_now_msec() - run;
        waste = (float)run / 1000;
        if (waste <= 0) {
            waste = 1;
        }
        avg = (all_file_size / waste) / 1024;
        printf("send file finish, time waste: %.1fs, avg send rate: %.1fKB/s\n", waste, avg);
    }
}
