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

off_t  all_file_size;

int
print_speed(char *file_name, uint64_t sum, uint64_t now) {
    double sum_t;
    double now_t;
    char *unit_n;
    char *unit_s;
    double sum_mb;
    double now_mb;
    static float rate = 0;
    static uint64_t move = 0;
    static uint64_t run_time = 0;

    if (run_time == 0) {
        run_time = get_now_msec();
        return 0;
    }
    
    if (now >= (1024 * 1024 * 2)) {
        unit_n = "MB";
        now_mb = (double)now / (1024 * 1024);
    } else {
        unit_n = "KB";
        now_mb = (double)now / 1024;
    }

    if (sum >= (1024 * 1024 * 2)) {
        unit_s = "MB";
        sum_mb = (double)sum / (1024 * 1024);
    } else {
        unit_s = "KB";
        sum_mb = (double)sum / 1024;
    }

    if (get_now_msec() - run_time >= 1000) {
        rate =((float) ((float)(now - move) / (float)(get_now_msec() - run_time))) / 1024;
        move = now;
        rate *= 1000;
        run_time = get_now_msec();
    }

    printf("%s [%.2f%s]: %.2fKB/S  %.2f%s/%.2f%s \r", 
           file_name, (float)((float)now/(float)sum) * 100, "%", rate, now_mb, unit_n, sum_mb, unit_s);
    setbuf(stdin, NULL);
}

int
send_file(int sockfd, char *file_name, int block) {
    int  i, t, move;
    int  fd;
    off_t file_move = 0;
    off_t file_size = 0;
    struct stat file_stat;
    char buff[MAX_BUFF];

    assert(block <= MAX_BUFF);
 
    fd = open(file_name, O_RDONLY);
    if (fd < 0) {
        printf("open file error %s\n", strerror(errno));
        exit(0);
    }
        
    i = stat(file_name, &file_stat);
    if (i < 0) {
        printf("stat %s error %s\n", file_name, strerror(errno));
        exit(0);
    }

    file_size = file_stat.st_size;
    all_file_size = file_size;
    print_speed(NULL, 0, 0);
    
    while (file_move < file_size) {
        //i = sendfile(sockfd, fd, &file_move, file_size - file_move);
        move = 0;
        t = read(fd, buff, block);
        if (t <= 0) {
            if (errno == EINTR) {
                continue;
            } else {
                printf("read file error %s\n", strerror(errno));
                exit(0);
            }
        }
    RESEND:
        i = send(sockfd, buff + move, t - move, MSG_NOSIGNAL);
        if (i < 0) {
            if (errno == EINTR) {
                goto RESEND;
            } else {
                printf("send file error %s\n", strerror(errno));
                exit(0);
            }
        }
        
        move += i;
        file_move += i;
        if (move < t) {
            goto RESEND;
        }

        print_speed(file_name, file_size, file_move);
    }    
    printf("\n");
}

int 
recv_file(int sockfd, char *temp) {
    int fd;
    int i;
    char buff[MAX_BUFF];

    fd = open(temp, O_RDWR | O_CREAT | O_TRUNC, 0666);
    if (fd < 0) {
        printf("open file error %s\n", strerror(errno));
        exit(0);
    }

    while(1) {
        i = recv(sockfd, buff, MAX_BUFF, 0);
        if (i <= 0) {
            if (errno == EINTR) {
                continue;
            } 
            printf("recv %s\n", strerror(errno));
            if (errno != 0) {
                exit(0);
            }
            break;
        }
        
        write(fd, buff, i);
    }
   
}

int
send_name(int sockfd, char *file_name) {
    char proto[PROTO_LEN + PROTO_FLAG + PROTO_NAME];
    int  len;
    int  i;
    int  move;

    memset(proto, '\0', PROTO_LEN + PROTO_FLAG + PROTO_NAME);
    init_proto(file_name, proto, &len);
    
    move = 0;
    
    while(move < len) {
        i = send(sockfd, proto + move, len - move, MSG_NOSIGNAL);
        if (i <= 0) {
            if (errno == EINTR) {
                continue;
            }
            printf("send %s\n", strerror(errno));
            exit(0);
        }
        move += i;
    }
    return 0;
}

int
parse_name(int sockfd, char *file_name) {
    uint32_t len = 4;
    uint32_t idx = 0;
    int i;
    char proto[PROTO_LEN + PROTO_FLAG + PROTO_NAME];

    memset(proto, '\0', PROTO_LEN + PROTO_FLAG + PROTO_NAME);
    
    while(1) {
        i = recv(sockfd, proto + idx, len - idx, 0);
        if (i <= 0) {
            if (i == EINTR) {
                continue;
            }
            printf("recv %s\n", strerror(errno));
            exit(0);
        }
        idx += i;
        if (idx < len) {
            continue;
        } else if (len == 4) {
            len = bytes_to_ui32(proto, 1);
            continue;
        } else {
            break;
        }
    }

    parse_proto(file_name, proto);
}


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
           "-s/--send    (send)send file mode\n"
           "-r/--recv    (recv)recv file mode\n"
           "-c/--client    (client)client mode\n"
           "-a/--server    (accept)server mode\n"
           "-f/--file [file]   (file)Send file(If -s is set then this must be set)\n"
           "-i/--ip [ip]     (ip)Set conn or listen ip\n"
           "-p/--port [port]   (port)Set conn or listen port\n"
           "-v/-h/--help/--version   (version/help)\n\n",
           VERSION
           );
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
    int   sockfd;

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
