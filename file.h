#ifndef _H_FILE_
#define _H_FILE_

int print_speed(char *file_name, uint64_t sum, uint64_t now);
int send_file(int sockfd, char *file_name, int block);
int recv_file(int sockfd, char *temp);
int send_name(int sockfd, char *file_name);
int parse_name(int sockfd, char *file_name);


#endif
