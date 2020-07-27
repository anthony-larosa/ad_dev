#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <unistd.h>
#include <termios.h>

#include <sys/ioctl.h>

#include <sys/dir.h>
#include <sys/types.h>
#include <sys/time.h>



#include <curl/curl.h>
#include <fcntl.h>
#include <errno.h>

#include "aero_time.h"
#include "my_com_port.h"

char *hostname = "https_aeronet"; 

int main(int argc, char **argv)
{
	MY_COM_PORT my_ports[50];
	int num, max_fd, retval;
	TIME_POINT current_time, aeronet_time;
        int iargc, ii;
        char **iargv;
	
	get_now(&current_time);
        printf_sys_time_with_title ("PC Time = ", &current_time);
       if (receive_aeronet_time(&aeronet_time))
 printf_sys_time_with_title ("AERONET Time = ", &aeronet_time);
        

        if (argc == 1) {

           iargc = 29;
         iargv = (char **)malloc(sizeof(char *) * iargc);
         for (ii = 1; ii < iargc; ii++)
           {
            iargv[ii] = (char *) malloc (10);
            sprintf (iargv[ii], "S%d", ii - 1);
          }

          num = open_group_of_com_ports (my_ports, iargc, iargv,&max_fd);
           
         }
else
	
	num = open_group_of_com_ports (my_ports, argc, argv,&max_fd);
	
        printf ("Hostname: %s\n", hostname);



	while (1)
	{
		
		retval = polling_group_of_com_ports(my_ports,  num, max_fd);
			if (retval == -1)	{
				send_group_of_com_ports(my_ports, num,  &current_time);
				printf("retval -1 \n");
			} 
			else
				if (retval == 1)
				{
					//printf("retval is 1 \n" );
					if (check_group_of_com_ports (my_ports, num, &current_time))
					{
						send_group_of_com_ports(my_ports, num,  &current_time);
						//printf("passed send 1 of comm ports\n"); 
						save_group_of_com_ports (my_ports, num ,&current_time);
						//printf("saving group of com ports \n"); 
						save_all_current_files_to_disk(my_ports, num, &current_time);
						//printf("saving all current files \n");
					}
					
				}
	}
	
}
	
