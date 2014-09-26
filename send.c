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
#include <getopt.h>

#define VERSION "0.1.1"

#define DEFAULT_IP   "113.10.136.179"
#define DEFAULT_PORT 6500
#define SEND_BLOCK   128
#define MAX_BUFF     2048

#define SEND_MODE    1
#define RECV_MODE    2

#define CONN_MODE    3
#define SERV_MODE    4

off_t  all_file_size;

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

    printf("%s [%.2f\%]: %.2fKB/S  %.2f%s/%.2f%s \r", 
           file_name, (float)((float)now/(float)sum) * 100, rate, now_mb, unit_n, sum_mb, unit_s);
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

    fd = open(temp, O_RDWR | O_CREAT | O_TRUNC, 0644);
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
            exit(0);
        }
        
        write(fd, buff, i);
    }
   
}

int 
accept_conn(char *ip, int port) {
    int sockfd;
    int i;
    struct sockaddr_in seraddr;
    
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
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

char *l_opt_arg;
struct option long_options[] = {
    
     { "send", 0, NULL, 's' },
    
     { "recv", 0, NULL, 'r' },
  
     { "file", 1, NULL, 'f' },
    
     { 0, 0, 0, 0 },
};

int
print_help() {
    printf(
           "Linux File Send/Recv Tool(FSRT)\n"
           "CopyRight By WinkChow\n"
           "Version:%s\n"
           "# You can set this tool to be a client or be a server.\n"
           "# It can be recv or send file when being a client or server.\n"
           "# Now only can support send or recv one file each time.\n"
           "# Recv and Send mode only one can be set.\n"
           "# Client and Server mode only one can be set.\n"
           "\n\n"
           "# Option:\n"
           "-s    (send)send file mode\n"
           "-r    (recv)recv file mode\n"
           "-c    (client)client mode\n"
           "-a    (accept)server mode\n"
           "-f [file]   (file)Send file(If -s is set then this must be set)\n"
           "-i [ip]     (ip)Set conn or listen ip\n"
           "-p [port]   (port)Set conn or listen port\n"
           "-v/-h   (version/help)\n\n",
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
    char  ip[32];
    int   port = 0;
    int   file_set = 0;
    int   mode = 0;
    int   connect_mode = 0;
    int   sockfd;

    memset(ip, '\0', 32);
    memset(file, '\0', 128);

    strncpy(ip, DEFAULT_IP, 32);
    port = DEFAULT_PORT;

    while((c = getopt(argc, argv, "vhsrcai:p:f:"))!= -1) {
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
        printf("connect mode not set\n");
        exit(0);
    }

    if (mode == 0) {
        printf("send recv mode not set\n");
        exit(0);
    }

    if ((mode == SEND_MODE) && (file_set == 0)) {
        printf("file not set\n");
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
        send_file(sockfd, file, SEND_BLOCK);
        break;
    case RECV_MODE:
        recv_file(sockfd, "temp.file");
        break;
    }

    run = get_now_msec() - run;
    waste = (float)run / 1000;
    avg = (all_file_size / waste) / 1024;
    printf("send file finish, time waste: %.1fs, avg send rate: %.1fKB/s\n", waste, avg);
}
