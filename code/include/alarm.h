// Alarm header.

#ifndef _ALARM_H_
#define _ALARM_H_

void alarmHandler(int signal);
int setAlarm(int timeout);

extern int alarm_count;
extern int alarm_enabled;

#endif // _ALARM_H_