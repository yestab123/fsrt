#ifndef _H_TOOL_
#define _H_TOOL_

#define PROTO_LEN  4
#define PROTO_FLAG 2
#define PROTO_NAME 256

#define COMPARE_FLAG 0xA5


int init_proto(char *file_name, char *proto, int *llen);
int parse_proto(char *file_name, char *proto);
char *get_file_name(char *file_name);
int set_reuseaddr(int fd);
uint32_t get_now_sec();
uint64_t get_now_msec();
#endif
