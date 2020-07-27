#ifndef _AERO_TIME_H_
#define _AERO_TIME_H_ 1


#include <stdio.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>

#include <sys/types.h>
#include <math.h>

typedef struct {
char day, month, year, hour, minute, second;
} TIME_POINT;

void get_GMT();
double get_julian();
int get_monday();
void get_now();
void increase_time();
void increase_time_day();
void time_copy();
int time_day_difference();
double time_difference();
void increase_time_month();
int time_month_difference();

#endif
