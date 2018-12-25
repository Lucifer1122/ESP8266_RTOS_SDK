/*
 * usbDevice.c
 *
 *  Created on: Dec 9, 2015
 *      Author: guozhixin2
 */
#include "common/thread.h"
#include "common/common.h"
#include <ctype.h>
#include <sys/un.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <linux/types.h>
#include <linux/netlink.h>
#include <errno.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <termios.h>
#include <dirent.h>
#include <getopt.h>
#include <fcntl.h>
#include "Function_Num.h"
#include "AttachFunc/LocalStorage/record_ini.h"
#include "time_lapse_func.h"
#include "common/common_config.h"
#include <stdlib.h>

#define UEVENT_BUFFER_SIZE 2048

int init_hotplug_sock()
{
	if(access(TIME_LINE_FILE,F_OK) == 0){
		time_line_Open = 1;
		time_line_ge = read_profile_int("Time","minute",60,TIME_LINE_FILE);
	}

    const int buffersize = 1024;
    int ret;

    struct sockaddr_nl snl;
    bzero(&snl, sizeof(struct sockaddr_nl));
    snl.nl_family = AF_NETLINK;
    snl.nl_pid = getpid();
    snl.nl_groups = 1;

    int s = socket(PF_NETLINK, SOCK_DGRAM, NETLINK_KOBJECT_UEVENT);
    if (s == -1)
    {
        perror("socket");
        return -1;
    }
    setsockopt(s, SOL_SOCKET, SO_RCVBUF, &buffersize, sizeof(buffersize));

    ret = bind(s, (struct sockaddr *)&snl, sizeof(struct sockaddr_nl));
    if (ret < 0)
    {
        perror("bind");
        close(s);
        return -1;
    }

    return s;
}

void read_dir(char* path,char *repth, char* depth)
{
    DIR* dirp =NULL;
    struct dirent* direntp=NULL;

    dirp = opendir(path);
    if( dirp == NULL)
        return;

    if(depth == NULL)
    	return;

    while((direntp = readdir(dirp)) !=NULL)
    {

    	if(strcmp(direntp->d_name,".")==0 || strcmp(direntp->d_name,"..")==0)    ///current dir OR parrent dir
    	   continue;

        if(direntp->d_type == DT_REG)
        {
          //printf("%s%s\n",depth, direntp->d_name);
        }
        else
        {
          //printf("%s\n", direntp->d_name);
          if(strstr(direntp->d_name,repth) != NULL){
        	  if(strncmp(repth,"ttyUSB",6) == 0){
        		  strcpy(depth,path);
        		  strcat(depth,direntp->d_name);
        	  }
        	  else if(strncmp(repth,"mmcblk",6) == 0){
        		  if(strlen(direntp->d_name) > 7){
        			  strcpy(depth,direntp->d_name);
        		  }
        	  }
        	  else if(strncmp(repth,"sd",2) == 0){
        		  if(strlen(direntp->d_name) == 4){
        			  if(strlen(depth) < strlen(direntp->d_name)){
        				  strcpy(depth,direntp->d_name);
        			  }
					  printf("readSD Path %s\n",depth);
				  }
        	  }
          }
        }

    }

    closedir(dirp);
}

int Find_USB_(int hotplug_sock)
{
	int r;
	int ret = 0;
	fd_set fdset;
	struct timeval tv;
	char buf[UEVENT_BUFFER_SIZE * 2] = {0};

	memset(buf,'\0',UEVENT_BUFFER_SIZE * 2);
	tv.tv_sec = 0;
	tv.tv_usec = 10000;
	FD_ZERO (&fdset);
	FD_SET(hotplug_sock,&fdset);
	r = select(hotplug_sock + 1, &fdset, NULL, NULL, &tv);
	if(r>0){
	   recv(hotplug_sock, &buf, sizeof(buf), 0);
	   printf("get Str %s\n",buf);
	   if(strstr(buf,"add") != NULL){
		   ret = 1;
	   }
	   else if(strstr(buf,"remove") != NULL){
		   ret = 2;
	   }
	   else if(strstr(buf,"change") != NULL){
		   ret = 3;
	   }
	}

	return ret;
}

void *Usb_Device_Search_thread(void *arg)
{
	char sdName[20];
	char sdPath_[50];
	char exePath[50];
	DIR *dir=NULL;
	int space_num = 0;
	memset(sdName,'\0',20);
	memset(sdPath_,'\0',50);

	long MemNum = 0;
	int is_Save_log = 0;

	char acTmpBuf[300] = {0};
	char acMacAddr[16] = {0};
	int n = 0;
	FILE *pFile = NULL;
	char logSaveFile[50];
	DIR *fileDir = NULL;

	if(access(TIME_LINE_FILE,F_OK) == 0){
		time_line_Open = 1;
		time_line_ge = read_profile_int("Time","minute",60,TIME_LINE_FILE);
	}

	//thread_synchronization(&rUsb_device, &rChildProcess);

	int hotplug_sock = init_hotplug_sock();

	while(1){

		n = Find_USB_(hotplug_sock);

		if(n == 1){
			memset(sdName,'\0',10);
			read_dir("/dev/","sd",sdName);
			if(strlen(sdName) != 0){
				memset(sdPath_,'\0',50);
				sprintf(sdPath_,"/mnt/%s/",sdName);
				memset(exePath,'\0',50);
				if(access(sdPath_,F_OK) != 0){
					sprintf(exePath,"mkdir %s",sdPath_);
					system(exePath);
					memset(exePath,'\0',50);
				}
				sprintf(exePath,"mount -t vfat /dev/%s %s",sdName,sdPath_);
				system(exePath);
				memset(exePath,'\0',50);
				dir = opendir(sdPath_);
				if(dir != NULL){
					closedir(dir);
					set_SD_static(1);
					memset(sd_path,'\0',20);
					memcpy(sd_path,sdPath_,strlen(sdPath_));

					fileDir = opendir("/tmp/www");
					if(fileDir == NULL){
						system("cp -r /usr/bin/www /tmp/");
						memset(sdPath_,'\0',50);
						sprintf(sdPath_,"ln -s %s /tmp/www/SDPath",sd_path);
						printf("Open %s \n",sdPath_);
						system("rm -rf /tmp/www/SDPath");
						system(sdPath_);
					}else{
						closedir(fileDir);
						memset(sdPath_,'\0',50);
						sprintf(sdPath_,"ln -s %s /tmp/www/SDPath",sd_path);
						printf("Open %s \n",sdPath_);
						system("rm -rf /tmp/www/SDPath");
						system(sdPath_);
					}

					system("cp /usr/boa/boa.conf /tmp/");
					system("cp /usr/boa/mime.types /tmp/");
					system("killall -9 boa");
					system("cp -r /usr/www /tmp/");
					system("./usr/boa/boa");

					Time_lapse_Init_Func(sd_path);

					is_Save_log = 0;
					space_num = 60;
				}
			}
		}else if(n == 2){
			if(get_SD_static() == 1){
				memset(exePath,'\0',50);
				sprintf(exePath,"umount %s",sd_path);
				system(exePath);
				memset(sd_path,'\0',20);
				set_SD_static(0);
			}
		}

		if(get_SD_static() == 0){

		}else{
			if(strlen(sd_path)!= 0){
				if(space_num > 60){
					space_num = 0;
					GetStorageInfo(sd_path,&MemNum,3);
					set_sd_space(MemNum);
					printf("SD have free %ld\n",MemNum);
				}else{
					space_num++;
				}

				if((get_SD_Save_Log_Func() != 0)&&(is_Save_log == 0)){
					memset(acTmpBuf,'\0',300);
					memset(acMacAddr,'\0',16);
					get_mac_addr_value(acTmpBuf, &n);
					memcpy(acMacAddr, acTmpBuf+12, 12);

					memset(logSaveFile,'\0',50);
					sprintf(logSaveFile,"%slog_%s.txt",sd_path,acMacAddr);

					printf("logSaveFile  %s  \n",logSaveFile);

					log_save(logSaveFile);

					sync();

					sleep(5);

					//playwavfile(SPEAKER_MUSIC_SAVE_LOG_OK,-1);

					is_Save_log = 1;
				}

				Save_image_in_Path(sd_path);
			}
		}

		if(USB_isInster_Num == 1){
			if(strlen(USB_in_path) != 0){

				Save_image_in_Path(USB_in_path);
			}
		}


		sleep(1);
	}
}

int CreateUsbDeviceThread()
{
	int result = 0;
	result = thread_start(Usb_Device_Search_thread);
	return result;
}

