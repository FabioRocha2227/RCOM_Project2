// Application layer protocol implementation
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include "application_layer.h"
#include "link_layer.h"
#include "utils.h"
#include "alarm.h"
#include <stdlib.h>
#include <sys/stat.h>

unsigned char packet[PCK_SIZE + 30];

void applicationLayer(const char *serialPort, const char *role, int baudRate, int nTries, int timeout, const char *filename) {

    LinkLayer connectionParameters;
    strcpy(connectionParameters.serialPort, serialPort);
        
    if(strcmp(role,"rx") == 0) {
        connectionParameters.role = LlRx;
    }
    else if(strcmp(role,"tx") == 0) {
        connectionParameters.role = LlTx;
    }
    else {
        perror(role);
        exit(-1);
    }

    connectionParameters.baudRate = baudRate;
    connectionParameters.nRetransmissions = nTries;
    connectionParameters.timeout = timeout;

    printf("\n\t---- LLOPEN ----\n\n");


    if(llopen(connectionParameters) == -1) {
      perror("Error opening a connection using the port parameters");
      exit(-1);
    }

    if(connectionParameters.role == LlRx) {

        int file_size = 0;
        int size_rx = 0;

        printf("\n\t---- LLREAD ----\n\n");

        int bytes = llread(packet);
        int type,length,*value;

        if(packet[0] == CTRLSTART) {
            int tlv_size = 1;
            while(tlv_size < bytes) {
                tlv_size += tlv(packet + tlv_size, &type, &length, &value);
                if(type == 0){
                    file_size =* value;
                    //printf("File size: %d\n",file_size);
                }
            }

            FILE* file = fopen(filename, "wb");

            if(file == NULL) {
                perror("Cannot open the file\n");
                exit(-1);
            } 
            else {
                printf("Control packet received\n");
            }

            int final_seq_num = 0;

            while(size_rx < file_size) {

                int bytes;
                if((bytes = llread(packet)) == -1){
                    perror("llread\n");
                    exit(-1);
                }

                if(packet[0] == CTRLEND){
                    perror("CTRL_END failed\n");
                    exit(-1);
                }

                if(packet[0] == CTRLDATA){
                    if(bytes < 5) {
                        perror("CTRLDATA failed\n");
                        exit(-1);
                    }
                    else if(packet[1] != final_seq_num){
                        perror("final_seq_num failed\n");
                        exit(-1);
                    }
                    else{
                        unsigned long size = packet[3] + packet[2]*256;

                        if(bytes-4 != size) {
                            perror("Header deprecated\n");
                            exit(-1);
                        }

                        fwrite(packet + 4, 1, size, file);
                        size_rx += size;

                        printf("Packet %d received\n", final_seq_num);

                        final_seq_num++;
                    }
                }
            }
            
            int bytes_read = llread(packet);

            if(bytes_read == -1) {
                perror("llread\n");
                exit(-1);
            }
            else if(bytes_read < 1) {
                perror("Short packet\n");
                exit(-1);
            }
            
            if(packet[0] != CTRLEND){
                printf("CTRLEND failed\n");
            }
            else{
                printf("Received end packet\n");
                printf("Transmission ending...\n");
            }
            fclose(file);
        }
        else {
            perror("Start packet failed\n");
            exit(-1);
        }
    }
    else if(connectionParameters.role == LlTx) {
        FILE* file = fopen(filename, "rb");
        
        if(file == NULL) {
            perror("Error openning the file\n");
            exit(-1);
        } 

        struct stat st;
        int file_size = (stat(filename, &st) == 0) ? st.st_size : 0;

        packet[0] = CTRLSTART;
        packet[1] = 0;
        packet[2] = sizeof(long);

        printf("\n\t---- LLWRITE ----\n\n");

        *((long*)(packet + 3)) = file_size;

        if(llwrite(packet,10) == -1){
            perror("llwrite\n");
            exit(-1);
        }

        int bytes_tx = 0;
        unsigned char i = 0;
        do {
            unsigned long total_bytes;
            if(file_size-bytes_tx < PCK_SIZE) {
                total_bytes = fread(packet + 4, 1, file_size-bytes_tx, file);
            }
            else {
                total_bytes = fread(packet + 4, 1, PCK_SIZE, file);
            }

            packet[0] = CTRLDATA;
            packet[1] = i;
            packet[2] = total_bytes >> 8;
            packet[3] = total_bytes % 256;
            
            if(llwrite(packet, total_bytes + 4) == -1){
                perror("llwrite\n");
                exit(-1);
                break;
            }
            printf("Packet %i sent\n",i);
            bytes_tx += total_bytes;
            i++;
        }while(bytes_tx < file_size);
        
        packet[0] = CTRLEND;
        if(llwrite(packet,1) == -1){
            perror("llwrite\n");
            exit(-1);
        }
 
        fclose(file);

    }

    printf("\n\t---- LLCLOSE ----\n\n");
    
    if(llclose(TRUE) == -1) {
        perror("llclose");
        exit(-1);
    }
}
