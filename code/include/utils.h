#ifndef _UTILS_H_
#define _UTILS_H_

#include "link_layer.h"

#define F 0x7e
#define ESC 0x7d
#define ESC_F 0x5e
#define ESC_E 0x5d
#define A_T 0x03
#define A_R 0x01

#define HEADER_SIZE 4
#define PCK_SIZE 512
#define MAX_PCK_SIZE 128

#define CTRLDATA 1
#define CTRLSTART 2
#define CTRLEND 3
#define CVSET 0x03
#define CVDISC 0x0b
#define CVUA 0x07

extern int alarm_count;
extern int alarm_enabled;
extern int disc;
extern unsigned char buffer[128];
extern int fd;

int llopenR(int fd);

int llopenT(int fd, int nRetransmissions, int timeout);

int stuffing(const unsigned char *buf, int bufSize, unsigned char* dest, unsigned char *bcc);

void create_command_frame(unsigned char* buf, unsigned char control_value, unsigned char address);

int header_frame(unsigned char* framebuf, const unsigned char* data, unsigned int data_size, unsigned char address, unsigned char control_value);

int tlv(unsigned char *address, int* type, int* length, int** value);

int write_cycle(int size);

int control_handler(ControlV cv, int R_S);

#endif // _UTILS_H