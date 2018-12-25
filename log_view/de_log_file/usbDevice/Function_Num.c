/*
 * Function_Num.c
 *
 *  Created on: 2015-10-15
 *      Author: guozhixin2
 */

#include <pthread.h>
#include "Function_Num.h"
#include "common/inifile.h"
#include "common/common_config.h"

pthread_mutex_t Function_Num_Lock = PTHREAD_MUTEX_INITIALIZER;

volatile int time_line_Open = 0;

volatile int time_line_ge = 1;

volatile char is_USB_Save_log = 0;

volatile char is_SD_Save_log = 0;

int get_Time_line_num()
{
	if(time_line_Open == 1){
		return time_line_ge;
	}else{
		return 0;
	}
}

void set_Time_line_num(int lin_num)
{
	char systxt[50];
	if(lin_num == 0){
		memset(systxt,'\0',50);
		sprintf(systxt,"rm %s",TIME_LINE_FILE);
		system(systxt);
		pthread_mutex_lock(&Function_Num_Lock);
		time_line_Open = 0;
		time_line_ge = 1;
		pthread_mutex_unlock(&Function_Num_Lock);
	}else{
		memset(systxt,'\0',50);
		sprintf(systxt,"%d",lin_num);
		write_profile_string("Time","minute",systxt,TIME_LINE_FILE);
		pthread_mutex_lock(&Function_Num_Lock);
		time_line_Open = 1;
		time_line_ge = lin_num;
		pthread_mutex_unlock(&Function_Num_Lock);
	}
}

void init_Save_Log_Num()
{
	pthread_mutex_lock(&Function_Num_Lock);
	is_SD_Save_log = read_profile_int("Savelog","SD",0,SAVE_LOG_FILE);
	is_USB_Save_log = read_profile_int("Savelog","USB",0,SAVE_LOG_FILE);
	pthread_mutex_unlock(&Function_Num_Lock);
}

void set_USB_Save_Log_Func(char usb_save)
{
	pthread_mutex_lock(&Function_Num_Lock);
	is_USB_Save_log = usb_save;
	if(usb_save == 1){
		write_profile_string("Savelog","USB","1",SAVE_LOG_FILE);
	}else{
		write_profile_string("Savelog","USB","0",SAVE_LOG_FILE);
	}
	pthread_mutex_unlock(&Function_Num_Lock);
}

char get_USB_Save_Log_Func()
{
	return is_USB_Save_log;
}

void set_SD_Save_Log_Func(char sd_save)
{
	pthread_mutex_lock(&Function_Num_Lock);
	is_SD_Save_log = sd_save;
	if(sd_save == 1){
		write_profile_string("Savelog","SD","1",SAVE_LOG_FILE);
	}else{
		write_profile_string("Savelog","SD","0",SAVE_LOG_FILE);
	}
	pthread_mutex_unlock(&Function_Num_Lock);
}

char get_SD_Save_Log_Func()
{
	return is_SD_Save_log;
}
