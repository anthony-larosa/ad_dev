#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <unistd.h>
#include <termios.h>

#include <sys/ioctl.h>

#include <sys/dir.h>
#include <sys/stat.h>
#include <sys/types.h>



#include <curl/curl.h>
#include <fcntl.h>
#include <errno.h>

#include "aero_time.h"
#include "my_com_port.h"

size_t dataSize;



extern char *hostname;



size_t handle_aeronet_time_internally (unsigned char *buffer, size_t size, size_t nmemb, TIME_POINT *aeronet_time)
{
	
	size_t i;
	
	long aertime;
	
	size_t strsize = nmemb * size;
	char time_string[20];
	if (strsize > 15) return  0;
	
	for (i = 0 ; i < strsize; i++)
		time_string[i] = buffer[i];
	time_string[i] = '\0';
	
	aertime = atol(time_string);
	aeronet_time->day = 1;
	aeronet_time->month = 1;
	aeronet_time->year = 70;
	aeronet_time->hour = aeronet_time->minute = aeronet_time->second = 0;
	increase_time(aeronet_time,1./86400. * aertime);
	return nmemb * size;
	
}



int receive_aeronet_time(TIME_POINT *aeronet_time)
{
	
	CURL *curl;
	CURLcode res;
	
	curl = curl_easy_init ();
	if (!curl) 
	{ 
		printf ("Could not make curl\n");
		return 0;
	}
	curl_easy_setopt(curl, CURLOPT_URL, "https://aeronet.gsfc.nasa.gov/cgi-bin/aeronet_time");
	curl_easy_setopt(curl, CURLOPT_TIMEOUT, 3);
	
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, handle_aeronet_time_internally);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, aeronet_time);
	
	res = curl_easy_perform(curl);
	if (res != CURLE_OK) {
		printf ("Setopt returned %d\n", res);
		curl_easy_cleanup(curl);
		return 0;
	}
	curl_easy_cleanup(curl);
	
	return 1;
}


void put_sys_time_to_buffer (TIME_POINT *sys_time, unsigned char *buffer)
{
	buffer[0] = sys_time->year;
	if (sys_time->month > 12) return;
	buffer[1] = sys_time->month;
	if (sys_time->day > 31) return;
	buffer[2] = sys_time->day;
	if (sys_time->hour > 23) return;
	buffer[3] = sys_time->hour;
	if (sys_time->minute > 59) return;
	buffer[4] = sys_time->minute;
	if (sys_time->second > 59) return;
	buffer[5] = sys_time->second;
}

void get_sys_time_from_buffer (TIME_POINT *sys_time, unsigned char *buffer)
{
	sys_time->year = buffer[0];
	sys_time->month = buffer[1];
	sys_time->day = buffer[2];
	sys_time->hour = buffer[3];
	sys_time->minute = buffer[4];
	sys_time->second = buffer[5];
	
}


void printf_sys_time_with_title (char *text_title, TIME_POINT *sys_time)
{
	printf ("%s : %02d:%02d:%d,%02d:%02d:%02d\n", 
		text_title,
		sys_time->day, sys_time->month, sys_time->year + 1900, sys_time->hour, sys_time->minute, sys_time->second);
}


void sprintf_sys_time_with_title (char *info_string, char *text_title, TIME_POINT *sys_time)
{
	sprintf (info_string,"%s : %02d:%02d:%d,%02d:%02d:%02d", 
		text_title,
		sys_time->day, sys_time->month, sys_time->year + 1900, sys_time->hour, sys_time->minute, sys_time->second);
}

unsigned char  convert_char_to_BYTE (unsigned char ch)
{
	if (ch < 48) return 0;
	if (ch < 58) return ch - 48;
	if (ch < 65) return 0;
	if (ch < 71) return ch - 55;
	return 0;
}



unsigned char get_decimall(unsigned char ch)
{
	
	return  (ch /16) * 10 +  ch % 16;
	
}







unsigned char convert_block(unsigned char *buffer, unsigned char *result, int num)
{
	unsigned char *buf = buffer, *res = result, *stop_byte = buffer + num;
	
	while (buf != stop_byte)
	{
		*res = 16 * convert_char_to_BYTE (buf[0]) + convert_char_to_BYTE (buf[1]);
		
		res++;
		buf+=2;
	}
	return result[0];
}


int get_cimel_time(unsigned char *buffer, TIME_POINT *cimel_time, int length)
{
	
	
	if (length == 10) {
		
		cimel_time->second = 255;
		cimel_time->hour = (buffer[0] - 48) * 10 + buffer[1] - 48;
		cimel_time->minute = (buffer[2] - 48) * 10 + buffer[3] - 48;
		
		cimel_time->day = (buffer[4] - 48) * 10 + buffer[5] - 48;
		cimel_time->month = (buffer[6] - 48) * 10 + buffer[7] - 48;
		cimel_time->year = (buffer[8] - 48) * 10 + buffer[9] - 48 + 100;
		return 0;
	}
	
	else if (length == 12)
	{
		
		cimel_time->hour = (buffer[0] - 48) * 10 + buffer[1] - 48;
		cimel_time->minute = (buffer[2] - 48) * 10 + buffer[3] - 48;
		cimel_time->second = (buffer[4] - 48) * 10 + buffer[5] - 48;
		
		cimel_time->day = (buffer[6] - 48) * 10 + buffer[7] - 48;
		cimel_time->month = (buffer[8] - 48) * 10 + buffer[9] - 48;
		cimel_time->year = (buffer[10] - 48) * 10 + buffer[11] - 48 + 100;
		return 1;
	}
	return 0;
}




void record_buffer_copy(RECORD_BUFFER *rec1, RECORD_BUFFER *rec2)
{
	rec1->buffer = (unsigned char *)malloc(rec2->record_size);
	rec1->idbyte = rec2->idbyte;
	time_copy(&rec1->record_time, &rec2->record_time);
	rec1->record_size = rec2->record_size;
	memcpy(rec1->buffer,rec2->buffer,rec2->record_size);
	strcpy(rec1->extens,rec2->extens);
}



void add_k7_buffer_to_k7_buffer(K7_BUFF *k7b1, K7_BUFF *k7b2)
{
	RECORD_BUFFER *new_records;
	int i, big_num;

	if (!k7b2->num_records) return;

	if (!k7b1->num_records)
	{
		k7b1->num_records = k7b1->allocated_records = k7b2->num_records;
		k7b1->rec = k7b1->records = (RECORD_BUFFER *)malloc(sizeof(RECORD_BUFFER) * k7b1->num_records);

		memcpy(&k7b1->the_header,&k7b2->the_header,sizeof(K7_HEADER));
		k7b1->empty_event_count = k7b2->empty_event_count;

		for (i = 0; i < k7b1->num_records; i++)
			record_buffer_copy(k7b1->records + i, k7b2->records + k7b1->num_records - i - 1);

		return;
	}

	big_num = k7b1->num_records + k7b2->num_records;
	new_records = (RECORD_BUFFER *)malloc(sizeof(RECORD_BUFFER) * big_num);

			for (i = 0; i < k7b1->num_records; i++)
			{
			record_buffer_copy(new_records + i, k7b1->records +  i);

			free(k7b1->records[i].buffer);
			}

			free (k7b1->records);

		    for (i = 0; i < k7b2->num_records; i++)
			record_buffer_copy(new_records + i + k7b1->num_records, k7b2->records + k7b2->num_records - i - 1);

			k7b1->rec = k7b1->records = new_records;
			k7b1->num_records = k7b1->allocated_records = big_num;

}

void free_k7_buffer (K7_BUFF *k7b)
{
	int i;
	
	if (!k7b->allocated_records) return;

	for (i = 0; i < k7b->num_records; i++)
		free(k7b->records[i].buffer);

	free (k7b->records);

	k7b->num_records = k7b->allocated_records = k7b->empty_event_count = 0;
}






int open_group_of_com_ports (MY_COM_PORT *com_ports, int argc, char **argv, int *max_fd)
{
	
	struct termios options;
	int i, num = 0, maxfd = 0;
	
	
	i = 0;
	
	while (i < argc - 1)
	{
		sprintf (com_ports[num].port_name,"/dev/tty%s", argv[i + 1]);
		printf ("Trying to open port %s\n", com_ports[num].port_name);
		
		com_ports[num].fd = open (com_ports[num].port_name, O_RDWR | O_NOCTTY | O_NDELAY);
		
		if (com_ports[num].fd > 0)
		{
			fcntl (com_ports[num].fd, F_SETFL, 0);
			
			
			
			tcgetattr (com_ports[num].fd,&options);
			
			cfsetspeed(&options, B1200);
			cfmakeraw (&options);
			//options.c_cflag &= ~CRTSCTS;
                        //options.c_lflag &= ~ICANON;

                       //:1 options.c_cflag |=  CREAD ;

			tcsetattr(com_ports[num].fd, TCSANOW, &options);

			tcflush(com_ports[num].fd, TCIOFLUSH);
			
			if (com_ports[num].fd > maxfd) maxfd = com_ports[num].fd;
			
			
			printf ("Port %s ooopen\n",com_ports[num].port_name);
			
			com_ports[num].dev_num = 0;
			com_ports[num].buf = com_ports[num].buffer;
			com_ports[num].begin = com_ports[num].end = NULL;
			com_ports[num].k7_buffer.num_records = 0;
			com_ports[num].k7_buffer.allocated_records = 20;
			com_ports[num].k7_buffer.rec = com_ports[num].k7_buffer.records = 
				(RECORD_BUFFER *)malloc (sizeof(RECORD_BUFFER) * com_ports[num].k7_buffer.allocated_records);
			
			//com_ports[num].k7_buffer.allocated_buffer = 40000;
			com_ports[num].k7_buffer.empty_event_count = 0;
			/*com_ports[num].k7_buffer.the_header.header_buffer = 
			com_ports[num].k7_buffer.buffer = 
				(unsigned char *)malloc(40000);*/
			//com_ports[num].k7_buffer.buf = com_ports[num].k7_buffer.buffer + 256;
			com_ports[num].stop_time.year = 90;
			com_ports[num].stop_time.month = 1;
			com_ports[num].stop_time.day = 1;
			com_ports[num].stop_time.hour = 0;
			com_ports[num].stop_time.minute = 0;
			com_ports[num].stop_time.second = 0;

			com_ports[num].k7_completed = 0;
			//com_ports[num].k7_buffer.buffer_size = 256;
			
			
			com_ports[num].header_flag = com_ports[num].time_header_flag = com_ports[num].data_flag_count = 
				com_ports[num].time_correction_flag = com_ports[num].time_count = 0;

			com_ports[num].k7_buffer_hourly.num_records = com_ports[num].k7_buffer_daily.num_records = 0;
			
			
			
			
			num++;
		}
		else printf ("Port %s can not open\n", com_ports[num].port_name);
		i++;
	}
	
	*max_fd = maxfd + 1;
	return num;
}

int polling_group_of_com_ports(MY_COM_PORT *com_ports, int num, int max_fd)
{
	
	fd_set rfds;
	struct timeval timeout;
	int i, retval, retnum = 0, read_bytes;
	unsigned char byte;
	
	FD_ZERO(&rfds);
	
	for (i = 0; i < num; i++)
	{
		FD_SET (com_ports[i].fd, &rfds);
		com_ports[i].if_flag = 0;
	}
	timeout.tv_sec = 1800;
	timeout.tv_usec = 0;
	retval = select (max_fd, &rfds, NULL, NULL, &timeout);
	if (retval == -1) {printf ("Error\n");exit(0);}
	
	if (retval == 0) 
	{
		printf ("No data in an Hour\n");
		return -1;
	}
	
	for (i =0; i < num; i++)
	{
		if (FD_ISSET(com_ports[i].fd, &rfds)) 
		{
			
			
			read_bytes = read(com_ports[i].fd,&byte, 1);
			
			if (read_bytes == 1) 
			{
				if ((com_ports[i].begin == NULL) && (byte == 2)) com_ports[i].begin = com_ports[i].buf;
				if ((com_ports[i].end == NULL) && (byte == 23)) com_ports[i].end = com_ports[i].buf;
				
				com_ports[i].buf[0] = byte;
				com_ports[i].buf++;
				com_ports[i].if_flag = 1;
				retnum = 1;
			}
			
			
			
			
		}
		
	}
	return retnum;
}



int check_buffer_for_checksum(MY_COM_PORT *mcport)
{
	
	unsigned char *buf, checksum, checks; 
	
//rintf ("\nChecksum shows :  %c%c\n", mcport->end[1], mcport->end[2]);
	
	
	
	convert_block(mcport->end + 1, &checksum, 2);
	
	buf = mcport->begin;
	checks = 0;
	
	while (buf != mcport->end + 1)
		checks ^= *buf++;
	
	return (checks == checksum);
}







int check_group_of_com_ports (MY_COM_PORT *com_ports, int num, TIME_POINT *current_time)
{
	MY_COM_PORT *mcport;
	K7_HEADER *the_header;
	K7_BUFF *k7_buffer;
	RECORD_BUFFER *record;
	unsigned char *header_buffer, *new_buffer;
	char time_correction_string[20];
	
	TIME_POINT aeronet_time, cimel_time, pc_time;
	
	int i, i_right, header_size, if_completed = 0, i_buf;
	for (i =0; i < num; i++)
		if (com_ports[i].if_flag) 
		{

//						printf ("i = %d  num_rec = %d\n", i, com_ports[i].k7_buffer.num_records);

			mcport = com_ports + i;
			if ((mcport->end != NULL) && (mcport->begin != NULL) && (mcport->buf - mcport->end == 3))
			{
				
				if (!check_buffer_for_checksum(mcport))
				{
					write (mcport->fd,"uT",2);
					i_right = 1;
				}
				else 
				{






				k7_buffer = &mcport->k7_buffer;
				the_header = &k7_buffer->the_header;
				header_buffer = the_header->header_buffer;
				record = k7_buffer->rec;
				i_right = 0;
				
				mcport->buffer_size = mcport->end - mcport->begin;
				
				/*printf ("Port %s - received block %d size  ciode = %c  time_count = %d\n",
					mcport->port_name, mcport->buffer_size, mcport->begin[1], mcport->time_count);
				*/
				
				switch (mcport->begin[1]) 
				{
				case 'S' :
					
					//printf ("Blin 1\n");
					
					if (!mcport->header_flag) 
					{
						
						if (!(mcport->buffer_size % 2))
						{
							
							header_size = (mcport->buffer_size - 12) /2;
							convert_block(mcport->begin + 12,header_buffer,mcport->buffer_size - 12);
							
							for (i = header_size; i < 256; i++)
								header_buffer[i] = 0;
							
							mcport->header_flag = 1;
							write (mcport->fd,"HT",2);
							i_right = 1;
						}
					}
					
					break;
					
				case 'h' :
				case 'H' :
				case 'R' :
					
					if (mcport->header_flag)
					{
						
						if (!mcport->time_header_flag) 
						{
							if (receive_aeronet_time(&aeronet_time))
							{
								the_header->internet_flag = 1;
								put_sys_time_to_buffer (&aeronet_time, header_buffer + 156);
							}
							else
							{
								header_buffer[156] = 
									header_buffer[157] = header_buffer[158] = 
									header_buffer[159] = header_buffer[160] = 
									header_buffer[161] = 255;
								the_header->internet_flag = 0;
							}
							the_header->correction_flag = get_cimel_time(mcport->begin + 2, &cimel_time, mcport->buffer_size - 2);
                                                        sprintf (header_buffer + 162,"%s", hostname);

							get_GMT(&pc_time);
							
							if (the_header->internet_flag && the_header->correction_flag) 
							{
								the_header->seconds_correction = 
									floor(time_difference(&aeronet_time, &cimel_time) * 86400);
								
								printf ("Suggested time correction : %d seconds\n",the_header->seconds_correction);
								if ((the_header->seconds_correction > 10) || (the_header->seconds_correction < -10))
									mcport->time_correction_flag = 1;
								
							}
							else the_header->seconds_correction = -9999;
							
							
							
							put_sys_time_to_buffer (&cimel_time, header_buffer + 144);
							put_sys_time_to_buffer (&pc_time, header_buffer + 150);
							
							the_header->cimel_number = header_buffer[3] * 256 + header_buffer[4];
							for (i = 0; i < 8; i++)
								the_header->eprom[i] = header_buffer[i + 128];
							the_header->eprom[8] = '\0';

							printf ("Port %s - header %s  %d\n", mcport->port_name, the_header->eprom, the_header->cimel_number);
							

							if (mcport->dev_num > 0)
							//if  ((mcport->dev_num > 0) && (mcport->dev_num != the_header->cimel_number))
							{
								send_hourly_file_now (mcport, current_time);
								send_daily_file_now  (mcport, current_time);
								
								mcport->stop_time.year = 90;
								mcport->stop_time.month = 1;
								mcport->stop_time.day = 1;
								mcport->stop_time.hour = 0;
								mcport->stop_time.minute = 0;
								mcport->stop_time.second = 0;
							
								
							}
							
							mcport->dev_num = the_header->cimel_number;
							
							mcport->time_header_flag = 1;
							i_right = 1;
							write (mcport->fd,"jT",2);
						}
						else 
						{
							if (mcport->header_flag && mcport->time_header_flag && mcport->time_correction_flag) 
							{
								switch (mcport->time_count)
								{
								case 0:
								case 2:
									write (mcport->fd,"HT",2);
									i_right = 1;
									
									mcport->time_count++;
									break;
									
								case 1:
									get_cimel_time(mcport->begin + 2, &cimel_time, mcport->buffer_size - 2);
									if (receive_aeronet_time (&aeronet_time))
									{
										
										
										sprintf (time_correction_string ,"R1234%02d%02d%02d%02d%02d%02dT",
											aeronet_time.hour, aeronet_time.minute, aeronet_time.second,aeronet_time.day,aeronet_time.month, aeronet_time.year % 100);
										
										write (mcport->fd, time_correction_string, 18);

										//printf ("%s\n", time_correction_string);


									}
									else write (mcport->fd,"HT", 2);
									
									i_right = 1;
									mcport->time_count++;
									break;
									
								case 3:
									mcport->k7_completed = 1;
									if_completed = 1;
									break;
								}
							}
						}
					}
					
					break;
					
				case '1' :
					
					
					if (mcport->header_flag && mcport->time_header_flag) 
					{
						
						record->record_size = (mcport->buffer_size - 2) /2;
						i_buf = 1;
						record->buffer = new_buffer = (unsigned char *)malloc(record->record_size);
						convert_block(mcport->begin + 2, record->buffer, mcport->buffer_size - 2);
						
						if (record->buffer[1] == record->record_size)
						{
							
							if (record->buffer[record->record_size - 1] == record->record_size) 	
							{
								record->idbyte = record->buffer[0];
								switch (record->idbyte) 
								{
								case 128 : strcpy(record->extens,"STA"); break;
								case 222 : strcpy(record->extens,"NSU"); break;
								case 141 : strcpy(record->extens,"ALR"); break;
								case 142 : strcpy(record->extens,"ALL"); break;
								case 139 : strcpy(record->extens,"PPP"); break;
								case 221 : strcpy(record->extens,"NSK"); break;
								case 220 : strcpy(record->extens,"SSK"); break;
								case 223 : strcpy(record->extens,"SUN"); break;
								case 133 : strcpy(record->extens,"ALM"); break;
								case 134 : strcpy(record->extens,"PP1"); break;
								case 136 : strcpy(record->extens,"BLK"); break; 
								case 226 : strcpy(record->extens,"SKY"); break;
								case 243 : strcpy(record->extens,"PRS"); break;
								case 144 : strcpy(record->extens,"APD"); break;
								case 145 : strcpy(record->extens,"APG"); break;
									
								case 255:  k7_buffer->empty_event_count ++; break;
								default :	record->idbyte = 0;break;
								}
								
								if (record->idbyte) 

								{
									if (record->idbyte != 255)
										
									{
										record->record_time.year = get_decimall(record->buffer[4]) + 100;
										record->record_time.month = get_decimall(record->buffer[5]);
										record->record_time.day = get_decimall(record->buffer[6]);
										record->record_time.hour = get_decimall(record->buffer[7]);
										record->record_time.minute = get_decimall(record->buffer[8]);
										record->record_time.second = get_decimall(record->buffer[9]);

										printf ("Port %s -> %s %02d:%02d:%d,%02d:%02d:%02d\n",
											mcport->port_name, record->extens, record->record_time.day,
											record->record_time.month, record->record_time.year + 1900,
											record->record_time.hour, record->record_time.minute, record->record_time.second);
										
										
										if (!mcport->data_flag_count) 
											time_copy(&mcport->last_time,&record->record_time);
										
										if (time_difference(&mcport->stop_time, &record->record_time) < 1./100000.)
											k7_buffer->empty_event_count = 2;
										else
											
										{
										
											i_buf = 0;
											mcport->data_flag_count++;
											
											k7_buffer->rec++;
											k7_buffer->num_records++;
											
											if (k7_buffer->num_records == k7_buffer->allocated_records) 
											{
												
												k7_buffer->allocated_records += 20;
												
												k7_buffer->rec = (RECORD_BUFFER *)realloc(k7_buffer->records, sizeof(RECORD_BUFFER) * k7_buffer->allocated_records);
												k7_buffer->records = k7_buffer->rec;
												k7_buffer->rec = k7_buffer->records + k7_buffer->num_records;
												
												printf ("Reallocated num = %d  alloc = %d\n",k7_buffer->allocated_records,k7_buffer->num_records); 
												
											}
											






											//k7_buffer->buf += record->record_size;
											//k7_buffer->buffer_size += record->record_size;
										}
									}
									
									
									
									
									
									if (k7_buffer->empty_event_count == 2) 
									{
										
										if (mcport->time_correction_flag) 
										{
											write(mcport->fd,"HT", 2);
											i_right = 1;
										}
										else
										{
											mcport->k7_completed = 1;
											if_completed = 1;
										}
									}
									else 
									{
										write(mcport->fd,"jT", 2);										
										i_right = 1;
										
									}
								}
							}
						}

						if (i_buf) free(new_buffer);

					}
					
					break;
}					
}


mcport->begin = mcport->end = NULL;
mcport->buf = mcport->buffer;

if (!i_right) 

{
	//printf ("ZTZ\n");
	write (mcport->fd, "ZTZ", 3);
	mcport->header_flag = 
		mcport->time_header_flag = 
		mcport->data_flag_count = 
		mcport->time_correction_flag = 
		mcport->time_count = 0;
}

}

}

return if_completed;

}




size_t upload_k7_buffer_internally (unsigned char *buffer, size_t size, size_t nmemb, unsigned char *k7_buffer)
{


	size_t total = size * nmemb;

	if (total > dataSize) total = dataSize;



	printf ("dataSize = %d  total = %d  size = %d  nmemb = %d\n", dataSize, total, size, nmemb);


	if (total > 0)
	{


	memcpy(buffer,k7_buffer,total);
	memcpy(k7_buffer, k7_buffer + total, dataSize - total);
	dataSize -= total;
	}
	return total;
}




 
int upload_k7_buffer_to_https(K7_BUFF *k7_buffer,char *upload_name)
{
 
        CURL *curl;
        CURLcode res;
                K7_HEADER *the_header; 
 
                long int buffer_length;
 
        struct curl_httppost *post=NULL;
        struct curl_httppost *last= NULL;
 
        unsigned char *buffer, *buf;
        int i;
 
        curl = curl_easy_init ();
  
        if (!curl) return 0;
 
        the_header = &k7_buffer->the_header;
 
                    buffer_length = 256;
 
        for (i = 0; i < k7_buffer->num_records; i++)
                buffer_length += k7_buffer->records[i].record_size;
 
        buffer = (unsigned char *)malloc(buffer_length);
        memcpy(buffer,k7_buffer->the_header.header_buffer,256);
 
        buf = buffer + 256;
        for (i = 0; i < k7_buffer->num_records; i++)
        {
                memcpy(buf, k7_buffer->records[i].buffer, k7_buffer->records[i].record_size);
                buf += k7_buffer->records[i].record_size;
        }
 
 
        curl_formadd (&post, &last, 
        CURLFORM_COPYNAME, "uploaded_file", CURLFORM_COPYCONTENTS, upload_name, CURLFORM_END);
 
       curl_formadd (&post, &last,
        CURLFORM_COPYNAME, "uploaded_file", CURLFORM_BUFFER, upload_name,  CURLFORM_BUFFERPTR, buffer, 
        CURLFORM_BUFFERLENGTH, buffer_length, CURLFORM_CONTENTTYPE,"application/octet-stream",  CURLFORM_END);
 
        curl_easy_setopt(curl, CURLOPT_URL, "https://aeronet.gsfc.nasa.gov/cgi-bin/webfile_trans_auto");
        curl_easy_setopt(curl, CURLOPT_HTTPPOST, post);
 
        res = curl_easy_perform(curl);
        curl_easy_cleanup(curl); 
                       
                curl_formfree(post);
        free(buffer);
 
                if (res != CURLE_OK) 
                {
                        printf ("Setopt returned %d\n", res);
                        
                        return 0;
        }
 
                        
                        return 1;
}
 



int save_k7_buffer_to_disk(K7_BUFF *k7_buffer, char *upload_name)
{

	K7_HEADER *the_header;
	char sscctt[200];
	int i, dSize;
	FILE *out;

	unsigned char *buffer, *buf;

	the_header = &k7_buffer->the_header;
    dSize = 256;

	for (i = 0; i < k7_buffer->num_records; i++)
		dSize += k7_buffer->records[i].record_size;

	buffer = (unsigned char *)malloc(dSize);
	memcpy(buffer,k7_buffer->the_header.header_buffer,256);

	buf = buffer + 256;
	for (i = 0; i < k7_buffer->num_records; i++)
	{
		memcpy(buf, k7_buffer->records[i].buffer, k7_buffer->records[i].record_size);
		buf += k7_buffer->records[i].record_size;
	}

	out = fopen(upload_name,"w");
	if (out != NULL)
	{
	fwrite(buffer,1,dSize,out);
	free(buffer);			
	fclose(out);

	return 1;
	}

	free(buffer);			
return 0;



}








void send_hourly_file_now (MY_COM_PORT *mcport, TIME_POINT *current_time)
{
	
	char sscctt[200];
	
	if (!mcport->k7_buffer_hourly.num_records) return;
	sprintf (sscctt,"%s_%s_%d_%d%02d%02d_%02d.K7",
		mcport->port_name + 8,
		mcport->k7_buffer_hourly.the_header.eprom,
		mcport->k7_buffer_hourly.the_header.cimel_number,
		current_time->year + 1900, current_time->month, current_time->day, current_time->hour);
        printf ("send sscctt= %s\n", sscctt);	

	if (upload_k7_buffer_to_https(&mcport->k7_buffer_hourly, sscctt))
		printf ("File %s uploaded\n", sscctt);
	else 
	{ 
		printf ("File %s not uploaded, ", sscctt);
		if (save_k7_buffer_to_disk(&mcport->k7_buffer_hourly,sscctt))
			printf (" saved on disk\n");
		else printf (" Can not save, lost\n");
	}
	printf ("Free ...\n");
	free_k7_buffer(&mcport->k7_buffer_hourly);
        printf ("Freed \n");
}




void send_daily_file_now (MY_COM_PORT *mcport, TIME_POINT *current_time)
{
	
	char sscctt[200];
	
	if (!mcport->k7_buffer_daily.num_records) return;
	sprintf (sscctt,"%s_%s_%d_%d%02d%02d.K7",
		mcport->port_name + 8,
		mcport->k7_buffer_daily.the_header.eprom,
		mcport->k7_buffer_daily.the_header.cimel_number,
		current_time->year + 1900, current_time->month, current_time->day);
	printf ("Dsend sscctt= %s\n", sscctt);
	if (upload_k7_buffer_to_https(&mcport->k7_buffer_daily, sscctt))
		printf ("File %s uploaded\n", sscctt);
	else 
	{ 
		printf ("File %s not uploaded, ", sscctt);
		if (save_k7_buffer_to_disk(&mcport->k7_buffer_daily,sscctt))
			printf (" saved on disk\n");
		else printf (" Can not save, lost\n");
	}
	printf ("Free ...\n");
	free_k7_buffer(&mcport->k7_buffer_daily);
printf ("Freed \n");
}




void save_all_current_files_to_disk(MY_COM_PORT *mcport, int num, TIME_POINT *current_time)
{
	
	char current_file_name[200];
	
    		sprintf (current_file_name,"%s_%s_%d_%d%02d%02d.K7",
		mcport->port_name + 8,
		mcport->k7_buffer_daily.the_header.eprom,
		mcport->k7_buffer_daily.the_header.cimel_number,
		current_time->year + 1900, current_time->month, current_time->day);
		printf("%s\n ", current_file_name );
		if (save_k7_buffer_to_disk(&mcport->k7_buffer_daily, current_file_name))
		printf ("Saved current file %s\n", current_file_name);
}






void send_group_of_com_ports (MY_COM_PORT *com_ports, int num, TIME_POINT *current_time)
{
	
	TIME_POINT now;
	int i;
	get_now(&now);
	
	if (now.hour == current_time->hour)  return;
	for (i = 0 ; i < num; i++)
		send_hourly_file_now (com_ports + i, current_time);
	
    if (now.day == current_time->day)
	{
		time_copy(current_time, &now);
		return;
	}
	
	for (i = 0 ; i < num; i++)
		send_daily_file_now (com_ports + i, current_time);
	
	time_copy(current_time, &now);
	
}








void save_group_of_com_ports (MY_COM_PORT *com_ports, int num, TIME_POINT *current_time)
{
	
	int i;
	TIME_POINT now;
	MY_COM_PORT *mcport;
	
	K7_BUFF *k7_buffer;
	
	
	
	for (i = 0 ; i < num; i++)
		if (com_ports[i].k7_completed) 
		{
			mcport = com_ports + i;
			k7_buffer = &mcport->k7_buffer;
			
				time_copy(&mcport->stop_time,&mcport->last_time);
			
			
			
			
			add_k7_buffer_to_k7_buffer (&mcport->k7_buffer_hourly, &mcport->k7_buffer);
			add_k7_buffer_to_k7_buffer (&mcport->k7_buffer_daily, &mcport->k7_buffer);
			
			
			free_k7_buffer(k7_buffer);
			
			
			k7_buffer->allocated_records = 20;
			k7_buffer->rec = k7_buffer->records = 
				(RECORD_BUFFER *)malloc (sizeof(RECORD_BUFFER) * k7_buffer->allocated_records);
			
			
			mcport->k7_completed = 0;
		}
}







