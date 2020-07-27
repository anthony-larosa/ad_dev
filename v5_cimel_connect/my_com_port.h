#ifndef _MYCOMMPORT_H_

#define _MYCOMMPORT_H_ 1

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>

#include <curl/curl.h>

#include "aero_time.h"


typedef struct {
	unsigned char *buffer, idbyte;
	char extens[5];
	int record_size;
	TIME_POINT record_time;
} RECORD_BUFFER;

typedef struct {
	unsigned char header_buffer[256];
	int cimel_number;
	int internet_flag, correction_flag;
	int seconds_correction;
	char eprom[9];
} K7_HEADER;

typedef struct {
	RECORD_BUFFER *records, *rec;
	int num_records, allocated_records, empty_event_count;
	//	, allocated_buffer, buffer_size;
	K7_HEADER the_header;
	//unsigned char *buffer, *buf;
} K7_BUFF;




typedef struct {
	int fd;
	char port_name[100];
	int buffer_size,
		if_flag, header_flag, time_header_flag, data_flag_count, 
		time_correction_flag, time_count, dev_num, 
		aero_connect, aero_reconnect, k7_completed;
	TIME_POINT stop_time, last_time;
	unsigned char buffer[2000], *begin, *end, *buf;
	K7_BUFF k7_buffer, k7_buffer_hourly, k7_buffer_daily;
	
	
} MY_COM_PORT;


void put_sys_time_to_buffer ();
void get_sys_time_from_buffer ();
void printf_sys_time_with_title ();
void sprintf_sys_time_with_title ();
unsigned char convert_char_to_BYTE (unsigned char ch);
unsigned char get_decimall(unsigned char ch);
unsigned char convert_block();
int get_cimel_time();
int receive_aeronet_time();
int open_group_of_com_ports();
int polling_group_of_com_ports();
int check_group_of_com_ports ();

void record_buffer_copy();
void free_k7_buffer ();
void add_k7_buffer_to_k7_buffer();
int save_k7_buffer_to_disk();
void send_daily_file_now ();
void send_hourly_file_now ();
void save_all_current_files_to_disk();
void send_group_of_com_ports ();
void save_group_of_com_ports ();



#endif


