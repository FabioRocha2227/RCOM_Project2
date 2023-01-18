#include <stdio.h>
#include <string.h>
#include "state_machine.h"
#include "link_layer.h"
#include "utils.h"

unsigned char address;
unsigned char control_value;
unsigned char bcc;
unsigned char *data;
unsigned int size;
FState fstate;

void state_machine(unsigned char byte){
    switch (fstate){
        case REJECT:
        case END:
            fstate = START;
        case START:
            //printf("START\n");
            if(byte == F)  {
                fstate = FLAG;
            }
            break;

        case FLAG:
            //printf("FLAG\n");
            size = 0;
            if(byte != F) {
                if(byte == A_T || byte == A_R){
                    fstate = ADDRESS;
                    address = byte;
                }
                else {
                    fstate = START;
                }
            }
            break;

        case ADDRESS:
            //printf("ADDRESS\n");
            if(byte == F) {
                fstate = FLAG;
            }
            else if( byte == control_handler(CVREJ,FALSE) || byte == control_handler(CVRR,FALSE) || byte == control_handler(CVREJ,TRUE) || byte == control_handler(CVRR,TRUE) 
            || byte == control_handler(CVDATA,FALSE) || byte == control_handler(CVDATA,TRUE) || byte == CVDISC || byte == CVSET || byte == CVUA){
                control_value = byte;
                bcc = address ^ control_value;
                fstate = CONTROL;
            }
            else {
                fstate = START;
            }
            break;

        case CONTROL:
            //printf("CONTROL\n");
            if(byte == F) {
                fstate = FLAG;
            }
            else if(byte == bcc) {
                fstate = BCC1_OK;
            }
            else {
                fstate = START;
            }
            break;

        case BCC1_OK:
            //printf("BCC1_OK\n");
            if(byte == F){
                if(control_value == control_handler(CVDATA, FALSE) || control_value == control_handler(CVDATA, TRUE)) {
                    fstate = F;
                    break;
                }
                fstate = END;
            }
            else if(((control_value == control_handler(CVDATA, FALSE) || control_value == control_handler(CVDATA, TRUE)) && data != NULL)){
                size = 0;
                if(byte == ESC) {
                    bcc = 0;
                    fstate = ESCAPE;
                    break;       
                }
                data[size] = byte;
                size++;
                bcc = byte;
                fstate = SUCCESS;
            }
            else {
             fstate = START;
            }
            break;

        case BCC2_OK:
            //printf("BCC2_OK\n");
            if(byte == F) {
                fstate = END;
            }
            else if(byte == 0) {
                data[size] = bcc;
                size++;
                bcc = 0;
                if(byte == ESC) {
                    data[size] = bcc;    
                    size++;
                    bcc = 0;
                    fstate = ESCAPE;
                }
            }
            else {
                data[size] = bcc;
                size++;
                data[size] = byte;
                size++;
                bcc = byte;
                fstate = SUCCESS;
            }
            break;
            
        case SUCCESS:
            //printf("SUCCESS\n");
            if(byte == F) {
                fstate = REJECT;
            }
            else if(byte == ESC) {
                fstate = ESCAPE;
            }
            else if(byte == bcc) {
                fstate = BCC2_OK;
            }
            else {
                data[size] = byte;
                size++;
                bcc ^= byte;
            }
            break;
            
        case ESCAPE:
            //printf("ESCAPE\n");
            if(byte == F) {
                fstate = REJECT;
            }
            else if(byte == ESC_F) {
                if(bcc == F){
                    fstate = BCC2_OK;
                    break;
                }
                bcc ^= F;
                data[size] = F;
                size++;
                fstate = SUCCESS;
            }
            else if(byte == ESC_E) {
                if(bcc == ESC) {
                    fstate = BCC2_OK;
                    break;
                }
                bcc ^= ESC;
                data[size] = ESC;
                size++;
                fstate = SUCCESS;
            }
            else {
                fstate = START;
            }
            break;
    }
}