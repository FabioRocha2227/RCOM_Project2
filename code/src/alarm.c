#include <unistd.h>
#include <signal.h>
#include <stdio.h>
#include "link_layer.h"
#include "alarm.h"

int alarm_enabled = FALSE;
int alarm_count = 0;

void alarmHandler(int signal) {
    alarm_enabled = FALSE;
    alarm_count++;

    printf("Alarm Counter: %d\n", alarm_count);
}

int setAlarm(int timeout) {
    (void)signal(SIGALRM, alarmHandler);

    while (alarm_count <= timeout) {
        if (alarm_enabled == FALSE) {
            alarm(timeout);
            alarm_enabled = TRUE;
        }
    }

    printf("------------------------\n");
    printf("Operation ending\n");
    printf("------------------------\n");

    return 0;
}