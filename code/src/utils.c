#include "utils.h"
#include "state_machine.h"
#include "link_layer.h"
#include "alarm.h"
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

int llopenR(int fd) {
    fstate = START;
    alarm_count = 0;
    int set = FALSE, byte;

    while(set == FALSE) {
            
        if((byte = read(fd, buffer, MAX_PCK_SIZE)) == -1) {
            perror("Couldn't read (llopen)\n");
            exit(-1);
        }
        int i = 0;
        do {
            state_machine(buffer[i]);
            if(fstate == END && address == A_T) {
                if(control_value == CVSET) {
                    set = TRUE;
                }
                else if(control_value == CVDISC) {
                    disc = TRUE;
                    printf("DISC Received\n");
                    return -1;
                }
            }
            i++;
        }while(i < byte && set == FALSE);
    }

    if(set == TRUE) {
        printf("Set Received\n");
    }

    create_command_frame(buffer, CVUA, A_T);
    
    write(fd, buffer, 5);

    printf("UA Sent\n");

    return 1;
}

int llopenT(int fd, int nRetransmissions, int timeout) {
        fstate = START;
        alarm_count = 0;
        int ua = FALSE, byte;
        
        while(ua == FALSE && alarm_count < nRetransmissions) {
            alarm(timeout);
            alarm_enabled = TRUE;

            if(alarm_count > 0) {
                printf("Time-out\n");
            }

            create_command_frame(buffer, CVSET, A_T);
            
            printf("SET sent\n");
            
            write(fd, buffer, 5);
            
            while(alarm_enabled && ua == FALSE){

                if((byte = read(fd, buffer, MAX_PCK_SIZE)) == -1) {
                    printf("Couldn't read (llopen)\n");
                    return -1;
                }

                int i = 0;
                
                do {
                    state_machine(buffer[i]);
                    if(fstate == END && address == A_T && control_value == CVUA) {
                        ua = TRUE;
                    }
                    i++;
                }while(i < byte && ua == FALSE);
            }
        }
        
        if(ua == TRUE) {
            printf("UA Received\n");
            return 1;
            
        }
        return -1;
}

int stuffing(const unsigned char *buffer, int bufSize, unsigned char* oct, unsigned char *bcc){
    int size = 0, i = 0;
    do{
        if(bcc != NULL) {
            *bcc ^= buffer[i];
        }
        
        if(buffer[i] == ESC) {
            oct[size++] = ESC;
            oct[size++] = ESC_E;
            return size;
        }
        
        else if(buffer[i] == F) {
            oct[size++] = ESC;
            oct[size++] = ESC_F;
            return size;
        }

        oct[size++] = buffer[i];
        i++;
    }while(i < bufSize);

    return size;
}

void create_command_frame(unsigned char* buf, unsigned char control_value, unsigned char address){

    buf[0] = F;
    buf[1] = address;
    buf[2] = control_value;
    buf[3] = address ^ control_value;
    buf[4] = F;

}

int header_frame(unsigned char* buf, const unsigned char* data,unsigned int data_size, unsigned char address, unsigned char control_value){

    buf[0] = F;
    buf[1] = address;
    buf[2] = control_value;
    buf[3] = address ^ control_value;

    int new_size = 0, i = 0;
    unsigned char bcc = 0;

    do{
        new_size += stuffing(data + i, 1, buf + new_size + HEADER_SIZE, &bcc);
        i++;
    }while(i < data_size);

    new_size += stuffing(&bcc,1, buf + new_size + HEADER_SIZE, NULL);
    buf[new_size + HEADER_SIZE] = F;
    
    new_size += 5;
    
    return new_size;
}

int tlv(unsigned char *address, int* type, int* length, int** value){
    
    *type = address[0];
    *length = address[1];
    *value = (int*)(address + 2);

    return 2 + *length;
}

int write_cycle(int size) {
    int i = 0, wr;
    while(i < size) {
        if((wr = write(fd, giant_buf + i, size - i)) == -1) {
            perror("Couldn't write (llwrite).\n");
            exit(-1);
        }
        i += wr;    
    }
    return i;
}

int control_handler(ControlV cv, int R_S) {
    switch (cv) {
        case CVRR:
            if(R_S % 2 != 0) {
                return 0b10000101;
            }
            else {
                return 0b00000101;
            }
            break;
        case CVREJ:
            if(R_S % 2 != 0){
                return 0b10000001;
            }
            else {
                return 0b00000001;
            }
            break;
        case CVDATA:
            if(R_S % 2 != 0) {
                return 0b01000000;
            }
            else {
                return 0b00000000;
            }
            break;
        default:
            break;
    }
    return -1;
}
