
#ifndef __UTIL_H__
#define __UTIL_H__

#include "stdint.h"
#include "util/macro.h"

#define IO_BUF_SZ 512

#if 0
#ifndef MIN
#define MIN(a,b) (((a)<(b))?(a):(b))
#endif
#ifndef MAX
#define MAX(a,b) (((a)>(b))?(a):(b))
#endif
#endif

uint32_t    mr(uint32_t addr);
uint32_t    mw(uint32_t addr, uint32_t value);

char        to_upper(char myChar);
int         atoi(const char *p);
uint8_t     itoa(int32_t i, char *b);
uint32_t    htoi(char *myArray);
uint8_t     itoh(uint32_t v, char *buf, uint8_t bytes);
int32_t     str2int(const char *buf);
int         str2bytes(const char *in, uint8_t *out, int maxlen);
uint16_t    bytes2str(const char *in, uint8_t *out, uint16_t sz);
uint8_t     int2str(int32_t i, char *buf, uint8_t base);
void        strip_str(char *str);

inline uint8_t myisspace(char c);
inline uint8_t myisnewline(char c);
inline uint8_t myisalnum(char c);
inline uint8_t myisdigit(char c);

int         mystrncmp(const char* s1, const char *s2, uint32_t n);
uint16_t    mystrnlen(const char* s, uint16_t maxlen);
void*       mymemcpy(void* dest, const void* src, uint32_t count);
char*       mystrncpy(char *dest, const char *src, uint32_t n);
uint32_t    mysnprintf(char *str, uint32_t sz, const char *fmt, ...);
uint8_t     myisempty(char *str);
char*       mystrchr(const char *s, int c);
char*       mystrncat(char *dest, const char *src, uint32_t n);
uint8_t*    mymemchr(const uint8_t *s, uint8_t c, uint32_t n);

typedef uint16_t (*io_recv_func)(char *buf);
extern io_recv_func io_recv_str_ptr;

uint16_t io_recv_str(char *buf);

typedef void (*io_send_func)(const char *buf, uint16_t sz);
extern io_send_func io_send_str_ptr;

void io_send_str(const char *str, uint16_t n);
void io_send_str2(const char *str);
void io_send_str3(const char *str, uint8_t newline);
void io_send_str4(const char *str);
void io_send_int(int n);
void io_send_int2(const char *str, int n);
void io_send_hex(unsigned int h, uint8_t bytes);
void io_send_hex2(const char *str, uint32_t h);
void io_send_hex3(const char *str, int n, uint32_t h);
void io_send_hex4(const char *str, const uint8_t *bytes, uint32_t n);
void io_newline(void);
void io_prompt(uint8_t newline);
void io_echo(void);

#ifdef __arm__
#ifdef MY_UART
#define dbg_send_str(a,b) uart_send_str(a,b)
#define dbg_send_str2(a) uart_send_str2(a)
#define dbg_send_str3(a,b) uart_send_str3(a,b)
#define dbg_send_str4(a) uart_send_str4(a)
#define dbg_send_int(a) uart_send_int(a)
#define dbg_send_int2(a,b) uart_send_int2(a,b)
#define dbg_send_hex(a) uart_send_hex(a)
#define dbg_send_hex2(a,b) uart_send_hex2(a,b)
#define dbg_send_hex3(a,b,c) uart_send_hex3(a,b,c)
#define dbg_send_hex4(a,b,c) uart_send_hex4(a,b,c)
#else
#define dbg_send_str(a,b) 0
#define dbg_send_str2(a) 0
#define dbg_send_str3(a,b) 0
#define dbg_send_str4(a) 0
#define dbg_send_int(a) 0
#define dbg_send_int2(a,b) 0
#define dbg_send_hex(a) 0
#define dbg_send_hex2(a,b) 0
#define dbg_send_hex3(a,b,c) 0
#define dbg_send_hex4(a,b,c) 0
#endif
#else
#define dbg_send_str(a,b) printf("%s",a)
#define dbg_send_str2(a) printf("%s",a)
#define dbg_send_str3(a,b) printf(b ? "\n\r%s" : "%s",a)
#define dbg_send_str4(a) printf("\n\r%s",a)
#define dbg_send_int(a) printf("%d",a)
#define dbg_send_int2(a,b) printf("\n\r%s = %d",a,b)
#define dbg_send_hex(a) printf("0x%8X",a)
#define dbg_send_hex2(a,b) printf("\n\r%s = 0x%8X",a,b)
#define dbg_send_hex3(a,b,c) (void)
#define dbg_send_hex4(a,b,c) (void)
#endif
#ifdef dbg_send_str
#define dbg_newline() dbg_send_str("\n\r", 2)
#endif

#endif /* __UTIL_H__ */

