#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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
#include "model5_port.h"

int main(int argc, char **argv)
{

    MY_COM_PORT mcport;
    int retval, status, dev_init, iarg;
    AERO_EXCHANGE aerex;

    char backup_dir[500], last_time_file[500], *homedir = getenv("HOME"), file_nameh[400], file_named[400], file_namem[400];

    int upload_switch_m, upload_switch_h, upload_switch_d, upload_switch;

    K7_BUFFER k7b, k7bm, k7bh, k7bd;
    struct tm mtim;
    time_t pc_time, new_time, last_time, stop_time;

    if (argc < 2)
        exit(0);

    pc_time = time(NULL);
    printf("%s starts. System clock %sWill activate modem\n", argv[0], ctime(&pc_time));

    aerex.good_clock = 0;

    system("sudo hologram network connect");
    stop_time = time(NULL);
    printf("Modem connected after %d seconds\n", stop_time - pc_time);

    receive_aeronet_time(&aerex);

    printf("Will disconnect modem\n");

    pc_time = time(NULL);
    system("sudo hologram network disconnect");
    stop_time = time(NULL);
    printf("Modem disconnected after %d seconds\n", stop_time - pc_time);

    sprintf(last_time_file, "%s/last_time.k7", homedir);

    sprintf(backup_dir, "%s/backup", homedir);
    mcport.time_interval = 900; // default : 15 minutes

    mcport.if_open_port = 0;
    sprintf(mcport.port_name, "/dev/tty%s", argv[1]);
    if (argc > 2)
        for (iarg = 2; iarg < argc; iarg++)
        {
            if (!strncmp(argv[iarg], "dir=", 4))
                strcpy(backup_dir, argv[iarg] + 4);
            else if (!strncmp(argv[iarg], "int=", 4))
                mcport.time_interval = atoi(argv[iarg] + 4);
        }

    gethostname(mcport.hostname, 39);
    mcport.hostname[39] = '\0';

    strcpy(k7b.file_name, "last_time.k7");

    last_time = read_k7_buffer_from_disk(homedir, &k7b);

    if (last_time)
    {
        printf("Found last saved time as %s  Cimel number is %d  eprom = %s\n", ctime(&last_time), k7b.cimel_number, k7b.eprom);
        mcport.cimel_number = k7b.cimel_number;
        strcpy(mcport.eprom, k7b.eprom);
        mcport.last_time = last_time;

        pc_time = time(NULL);
        gmtime_r(&pc_time, &mtim);

        sprintf(k7bm.file_name, "%s_%04d_%d%02d%02d_%02d%02d.K7", k7b.eprom, k7b.cimel_number,
                mtim.tm_year + 1900, mtim.tm_mon + 1, mtim.tm_mday, mtim.tm_hour, mtim.tm_min);
        init_k7_buffer(&k7bm);
        combine_k7_buffers(&k7bm, &k7b);

        sprintf(k7bh.file_name, "%s_%04d_%d%02d%02d_%02d.K7", k7b.eprom, k7b.cimel_number,
                mtim.tm_year + 1900, mtim.tm_mon + 1, mtim.tm_mday, mtim.tm_hour);
        init_k7_buffer(&k7bh);

        sprintf(k7bd.file_name, "%s_%04d_%d%02d%02d.K7", k7b.eprom, k7b.cimel_number,
                mtim.tm_year + 1900, mtim.tm_mon + 1, mtim.tm_mday);
        read_k7_buffer_from_disk(backup_dir, &k7bd);

        free_k7_buffer(&k7b);
    }
    else
    {
        printf("Found no previously made last time file\n");
        mcport.cimel_number = -1;
        mcport.last_time = 0;
        init_k7_buffer(&k7bm);
        init_k7_buffer(&k7bh);
        init_k7_buffer(&k7bd);
        init_k7_buffer(&k7b);
    }

    strcpy(mcport.program_version, PROG_VERSION);

    mcport.packet_timeout = 15;

    open_my_com_port(&mcport);
    wait_for_new_packet(&mcport);
    init_port_receiption(&mcport);

    while (1)
    {

        retval = main_loop_cycle(&mcport, &aerex, &k7b);
        if (retval)
        {
            if (retval == 8)
            {

                if (mcport.cimel_number == -1)
                    dev_init = 1;
                else if ((mcport.cimel_number != k7b.cimel_number) || strcmp(mcport.eprom, k7b.eprom))
                {
                    save_k7_buffer_on_disk(backup_dir, &k7bd);
                    pc_time = time(NULL);
                    printf("Will upload existing file %s to aeronet, System clock %sWill activate modem\n", k7bd.file_name, ctime(&pc_time));
                    system("sudo hologram network connect");
                    stop_time = time(NULL);
                    printf("Modem connected after %d seconds\n", stop_time - pc_time);
                    libcurl_upload_k7_buffer_to_https(&k7bd);

                    receive_aeronet_time(&aerex);

                    printf("Will disconnect modem\n");

                    pc_time = time(NULL);
                    system("sudo hologram network disconnect");
                    stop_time = time(NULL);
                    printf("Modem disconnected after %d seconds\n", stop_time - pc_time);

                    free_k7_buffer(&k7bm);
                    free_k7_buffer(&k7bh);
                    free_k7_buffer(&k7bd);
                    dev_init = 2;
                }
                else
                    dev_init = 0;

                if (dev_init)
                {
                    printf("Redefined  Cimel number = %d  eprom = %s\n", k7b.cimel_number, k7b.eprom);
                    mcport.cimel_number = k7b.cimel_number;
                    strcpy(mcport.eprom, k7b.eprom);

                    pc_time = time(NULL);

                    gmtime_r(&pc_time, &mtim);
                    sprintf(k7bm.file_name, "%s_%04d_%d%02d%02d_%02d%02d.K7", k7b.eprom, k7b.cimel_number,
                            mtim.tm_year + 1900, mtim.tm_mon + 1, mtim.tm_mday, mtim.tm_hour, mtim.tm_min);
                    sprintf(k7bh.file_name, "%s_%04d_%d%02d%02d_%02d.K7", k7b.eprom, k7b.cimel_number,
                            mtim.tm_year + 1900, mtim.tm_mon + 1, mtim.tm_mday, mtim.tm_hour);
                    sprintf(k7bd.file_name, "%s_%04d_%d%02d%02d.K7", k7b.eprom, k7b.cimel_number,
                            mtim.tm_year + 1900, mtim.tm_mon + 1, mtim.tm_mday);
                }
            }
            else
            {
                if ((retval == 2) || (retval == 3))
                {
                    printf("k7 buffer downloaded num = %d\n", k7b.num_records);
                    save_k7_buffer_on_disk(homedir, &k7b);

                    combine_k7_buffers(&k7bm, &k7b);
                    combine_k7_buffers(&k7bh, &k7b);
                    combine_k7_buffers(&k7bd, &k7b);
                    save_k7_buffer_on_disk(backup_dir, &k7bd);
                    free_k7_buffer(&k7b);
                }

                if ((retval == 1) || (retval == 3))
                {

                    new_time = time(NULL);
                    printf("Interval reached system time %s", ctime(&new_time));

                    upload_switch = upload_switch_m = upload_switch_h = upload_switch_d = 0;

                    gmtime_r(&new_time, &mtim);
                    sprintf(file_namem, "%s_%04d_%d%02d%02d_%02d%02d.K7", mcport.eprom, mcport.cimel_number,
                            mtim.tm_year + 1900, mtim.tm_mon + 1, mtim.tm_mday, mtim.tm_hour, mtim.tm_min);
                    sprintf(file_nameh, "%s_%04d_%d%02d%02d_%02d.K7", mcport.eprom, mcport.cimel_number,
                            mtim.tm_year + 1900, mtim.tm_mon + 1, mtim.tm_mday, mtim.tm_hour);
                    sprintf(file_named, "%s_%04d_%d%02d%02d.K7", mcport.eprom, mcport.cimel_number,
                            mtim.tm_year + 1900, mtim.tm_mon + 1, mtim.tm_mday);

                    if (strcmp(file_namem, k7bm.file_name))
                    {

                        if (k7bm.if_header)
                            upload_switch_m = upload_switch = 2;
                        else
                            upload_switch_m = 1;
                    }

                    if (strcmp(file_nameh, k7bh.file_name))
                    {
                        if (k7bh.if_header)
                            upload_switch_h = upload_switch = 2;
                        else
                            upload_switch_h = 1;
                    }
                    if (strcmp(file_named, k7bd.file_name))
                    {
                        if (k7bd.if_header)
                            upload_switch_d = upload_switch = 2;
                        else
                            upload_switch_d = 1;
                    }

                    if (!upload_switch)
                        printf("There is nothing new to upload\n");
                    else
                    {
                        pc_time = time(NULL);
                        printf("Will activate modem\n", ctime(&pc_time));
                        system("sudo hologram network connect");
                        stop_time = time(NULL);
                        printf("Modem connected after %d seconds\n", stop_time - pc_time);

                        if (upload_switch_m == 2)
                        {
                            printf("Will upload %s\n", k7bm.file_name);
                            libcurl_upload_k7_buffer_to_https(&k7bm);
                        }
                        if (upload_switch_h == 2)
                        {
                            printf("Will upload %s\n", k7bh.file_name);
                            libcurl_upload_k7_buffer_to_https(&k7bh);
                        }
                        if (upload_switch_d == 2)
                        {
                            printf("Will upload %s\n", k7bd.file_name);
                            libcurl_upload_k7_buffer_to_https(&k7bd);
                        }

                        receive_aeronet_time(&aerex);

                        printf("Will disconnect modem\n");

                        pc_time = time(NULL);
                        system("sudo hologram network disconnect");
                        stop_time = time(NULL);
                        printf("Modem disconnected after %d seconds\n", stop_time - pc_time);
                    }

                    if (upload_switch_m)
                    {
                        free_k7_buffer(&k7bm);
                        strcpy(k7bm.file_name, file_namem);
                    }
                    if (upload_switch_h)
                    {
                        free_k7_buffer(&k7bh);
                        strcpy(k7bh.file_name, file_nameh);
                    }
                    if (upload_switch_d)
                    {
                        free_k7_buffer(&k7bd);
                        strcpy(k7bd.file_name, file_named);
                    }
                }
            }
        }
    }
}
