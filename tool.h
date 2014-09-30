#ifndef _H_TOOL_
#define _H_TOOL_

#define PROTO_LEN  4
#define PROTO_FLAG 2
#define PROTO_FILE 4
#define PROTO_NAME 256

#define COMPARE_FLAG 0xA5


int init_proto(char *file_name, char *proto, int file_size, int *llen);
int parse_proto(char *file_name, char *proto, int *file_size);
char *get_file_name(char *file_name);
int set_reuseaddr(int fd);
uint32_t get_now_sec();
uint64_t get_now_msec();
uint16_t bytes_to_ui16(const char *s, int from_big_endian);
uint32_t bytes_to_ui32(const char *s, int from_big_endian);
void ui16_to_bytes(char *s, uint16_t n);
void ui32_to_bytes(char *s, uint32_t n);
void ui8_to_bytes(char *s, uint8_t n);
uint8_t bytes_to_ui8(const char *s);
#endif
