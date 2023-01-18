// Statemachine header.

#ifndef _STATEMACHINE_H_
#define _STATEMACHINE_H_

extern unsigned char address;
extern unsigned char control_value;
extern unsigned char bcc;
extern unsigned char *data;
extern unsigned int size;

typedef enum {START, FLAG, ADDRESS, CONTROL, BCC1_OK, BCC2_OK, SUCCESS, ESCAPE, END, REJECT} FState;
extern FState fstate;

void state_machine(unsigned char byte);

#endif // _STATEMACHINE_H_