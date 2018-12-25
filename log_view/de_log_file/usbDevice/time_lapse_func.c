/*
 * time_lapse_func.c
 *
 *  Created on: 2015-10-28
 *      Author: guozhixin2
 */
#include <stdio.h>
#include <stdlib.h>
#include <sys/statfs.h>
#include <dirent.h>
#include "BaseServe/VideoCapture/video.h"
#include "common/common_config.h"
#include "AttachFunc/LocalStorage/record_ini.h"
#include "Function_Num.h"
#include "time_lapse_func.h"
#include "common/inifile.h"
#include "PlatFormServe/IoCtl/ioctl.h"

#define TIME_LAPSE_SETUP_FILE ".setup"

unsigned int iFrame_num = 0;
struct timeval stTimeOld;

pthread_mutex_t Time_lapse_Lock = PTHREAD_MUTEX_INITIALIZER;

volatile struct Time_lapse_struct timl;

volatile int TL_Name = 0;

volatile int Time_Day = 0;

void Time_lapse_Init_Func(char *Path)
{
	char fileSave[100];
	int i;
	DIR *fileDir = NULL;

	memset(fileSave,'\0',100);
	sprintf(fileSave,"%stime_lapse",Path);
	fileDir = opendir(fileSave);
	if(fileDir == NULL){
		mkdir(fileSave,0755);
	}else{
		closedir(fileDir);
	}

	memset(fileSave,'\0',100);
	sprintf(fileSave,"%stime_lapse/%s",Path,TIME_LAPSE_SETUP_FILE);
	char num[10];

	timl.Time_lapse_Num = read_profile_int("setup","name",0,fileSave);
	timl.TimeBegin = read_profile_int("setup","beginTime",0,fileSave);
	timl.TimeEnd = read_profile_int("setup","endTime",0,fileSave);
	timl.TimeGap = read_profile_int("setup","TimeGap",0,fileSave);
	timl.Task_Num = read_profile_int("setup","TaskNum",0,fileSave);
	timl.Time_zone = read_profile_int("setup","Timezone",0,fileSave);
	timl.Time_Pir_Gap = read_profile_int("setup","TimePirGap",0,fileSave);



	timl.TimeCha = read_profile_int("setup","TimeCha",0,fileSave);

	if((timl.Task_Num != 0)&&(timl.Task_Num < 10)){
		for(i=0;i<timl.Task_Num;i++)
		{
			memset(num,'\0',10);
			sprintf(num,"beg%d",i);
			timl.Task_begin[i] = read_profile_int("Task",num,0,fileSave);
			memset(num,'\0',10);
			sprintf(num,"end%d",i);
			timl.Task_end[i] = read_profile_int("Task",num,0,fileSave);
		}
	}

//	if(timl.Time_lapse_Num != 0){
//		if(timl.Time_Pir_Gap > 0){
//			if(timl.Time_Pir_Gap != timl.TimeGap){
//				//打开红外
//				pir_control(MSG_IOCTL_PIR_TIME_LAPSE);
//			}else{
//				pir_control(MSG_IOCTL_PIR_TIME_LAPSE_CLOSE);
//			}
//		}else{
//			pir_control(MSG_IOCTL_PIR_TIME_LAPSE_CLOSE);
//		}
//	}
}

int Time_lapse_Set_Func(struct Time_lapse_struct *timltmp,char *Path)
{
	pthread_mutex_lock(&Time_lapse_Lock);
	memcpy((char *)(&timl),(char *)timltmp,sizeof(struct Time_lapse_struct));
	pthread_mutex_unlock(&Time_lapse_Lock);

	char fileSave[100];
	memset(fileSave,'\0',100);
	char tmpStr[100];
	int i= 0;

	char num[10];

	sprintf(fileSave,"%stime_lapse/%s",Path,TIME_LAPSE_SETUP_FILE);

	printf("file Save %s \n",fileSave);

	if(timl.Time_lapse_Num == 0){
		memset(fileSave,'\0',100);
		sprintf(fileSave,"rm -f %stime_lapse/%s",Path,TIME_LAPSE_SETUP_FILE);
		printf("%s \n",fileSave);
		system(fileSave);
//		pir_control(MSG_IOCTL_PIR_TIME_LAPSE_CLOSE);
	}else{
		memset(tmpStr,'\0',100);
		sprintf(tmpStr,"%d",timl.Time_lapse_Num);
		write_profile_string("setup","name",tmpStr,fileSave);
		printf("setup name %d \n",timl.Time_lapse_Num);
		memset(tmpStr,'\0',100);
		sprintf(tmpStr,"%d",timl.Task_Num);
		write_profile_string("setup","TaskNum",tmpStr,fileSave);
		memset(tmpStr,'\0',100);
		sprintf(tmpStr,"%d",timl.TimeGap);
		write_profile_string("setup","TimeGap",tmpStr,fileSave);
		memset(tmpStr,'\0',100);
		sprintf(tmpStr,"%d",timl.TimeEnd);
		write_profile_string("setup","endTime",tmpStr,fileSave);
		memset(tmpStr,'\0',100);
		sprintf(tmpStr,"%d",timl.TimeBegin);
		write_profile_string("setup","beginTime",tmpStr,fileSave);
		memset(tmpStr,'\0',100);
		sprintf(tmpStr,"%d",timl.Time_zone);
		write_profile_string("setup","Timezone",tmpStr,fileSave);
		memset(tmpStr,'\0',100);
		sprintf(tmpStr,"%d",timl.Time_Pir_Gap);
		write_profile_string("setup","TimePirGap",tmpStr,fileSave);

//		if(timl.Time_Pir_Gap > 0){
//			if(timl.Time_Pir_Gap != timl.TimeGap){
//				//打开红外
//				pir_control(MSG_IOCTL_PIR_TIME_LAPSE);
//			}else{
//				pir_control(MSG_IOCTL_PIR_TIME_LAPSE_CLOSE);
//			}
//		}else{
//			pir_control(MSG_IOCTL_PIR_TIME_LAPSE_CLOSE);
//		}
		timl.TimeCha = (timl.TimeEnd - timl.TimeBegin)/86400;

		memset(tmpStr,'\0',100);
		sprintf(tmpStr,"%d",timl.TimeCha);
		write_profile_string("setup","TimeCha",tmpStr,fileSave);



		if((timl.Task_Num != 0)&&(timl.Task_Num < 10)){
			for(i=0;i<timl.Task_Num;i++)
			{
				memset(num,'\0',10);
				sprintf(num,"beg%d",i);
				memset(tmpStr,'\0',100);
				sprintf(tmpStr,"%d",timl.Task_begin[i]);
				write_profile_string("Task",num,tmpStr,fileSave);
				memset(num,'\0',10);
				sprintf(num,"end%d",i);
				memset(tmpStr,'\0',100);
				sprintf(tmpStr,"%d",timl.Task_end[i]);
				write_profile_string("Task",num,tmpStr,fileSave);
			}
		}
	}

	return 1;
}

int Time_lapse_Get_Func(struct Time_lapse_struct *timltmp)
{
	pthread_mutex_lock(&Time_lapse_Lock);
	memcpy((char *)timltmp,(char *)(&timl),sizeof(struct Time_lapse_struct));
	pthread_mutex_unlock(&Time_lapse_Lock);
	return 1;
}

int Time_lapse_Save_Image(char *imageName)
{
	int nRet = 0;
	struct video_node_t *pstVideobmp = NULL;
	FILE *bmpFlie = NULL;

	pstVideobmp = VC_alloc_frame(0, 0, VC_FRAME_MODE_CURRENT, &nRet);

	if ((0 == nRet) && (NULL != pstVideobmp)){
		pthread_mutex_lock(&Guo_Wirte_File_Lock);
		bmpFlie = fopen(imageName,"wb+");
		if (NULL != bmpFlie)
		{
			fwrite(pstVideobmp->map,
					pstVideobmp->len, 1, bmpFlie);
			fclose(bmpFlie);
			bmpFlie = NULL;
			printf("finish record and write to file ok  %s\n",imageName);
		}else{
			printf("File write faile %s \n",imageName);
		}
		pthread_mutex_unlock(&Guo_Wirte_File_Lock);
		VC_free_frame(0, pstVideobmp);
	}

}

int Time_lapse_Save_H264(char *VideoName)
{
	FILE *bmpFlie = NULL;

	if((video_iFrame.seq != 0)&&(video_iFrame.seq != iFrame_num)){
		iFrame_num = video_iFrame.seq;
		pthread_mutex_lock(&Guo_Wirte_File_Lock);
		bmpFlie = fopen(VideoName,"a+b");
		if (NULL != bmpFlie)
		{
			pthread_mutex_lock(&(video_iFrame.lock));
			fwrite(video_iFrame.videoiFrame,
					video_iFrame.len, 1, bmpFlie);
			pthread_mutex_unlock(&(video_iFrame.lock));
			fclose(bmpFlie);
			bmpFlie = NULL;
		}
		pthread_mutex_unlock(&Guo_Wirte_File_Lock);
		printf("finish record and write to file ok  %s\n",VideoName);
	}
}

void putLog(char *path,char *str)
{
	FILE *out;

	if(path == NULL)
		return;

	if(str == NULL)
		return;

	if(strlen(path) == 0){
		return;
	}

	out = fopen(path,"a+");
	if(NULL == out)
	{
		return;
	}

	if(-1 == fputs(str,out) )
	{
		fclose(out);
		return;
	}

	fclose(out);
}

void Save_image_in_Path(char *Path)
{
	char tim_file_name[100],tmpDay[100];
	char fileSave[100],logSave[100],logSaveTxt[100];
	DIR *fileDir = NULL;
	time_t now;
	struct tm *tm_now;
	int i = 0;
	struct timeval stTime;
	short tim_now_date;
	int GapTmp = 0;

	gettimeofday(&stTime, NULL);

	struct Time_lapse_struct timlthis;

	Time_lapse_Get_Func(&timlthis);

	if(timlthis.Time_lapse_Num != TL_Name){
		//在原任务log中，增加日志
		TL_Name = timlthis.Time_lapse_Num;
	}

	memset(tim_file_name,'\0',100);
	sprintf(tim_file_name,"%stime_lapse/time_Task_%d",Path,TL_Name);
	memset(logSave,'\0',100);
	sprintf(logSave,"%stime_lapse/time_Task_%d/log.txt",Path,TL_Name);

	if(stTime.tv_sec > 1400000000){
		if(TL_Name != 0){
			fileDir = opendir(tim_file_name);

			if(fileDir == NULL){
				mkdir(tim_file_name,0755);
				//增加初创日志

				memset(logSaveTxt,'\0',100);
				sprintf(logSaveTxt,"[BEGIN]\n[BEGINTIME]%d\n[ENDTIME]%d\n[TIMEGAP]%d\n",timlthis.TimeBegin,timlthis.TimeEnd,timlthis.TimeGap);
				putLog(logSave,logSaveTxt);

			}else{
				closedir(fileDir);
			}

			if(((timlthis.TimeEnd == 0)
					&&(timlthis.TimeBegin == 0)
					)||(
							(stTime.tv_sec+timlthis.Time_zone <= timlthis.TimeEnd)
							&&(stTime.tv_sec+timlthis.Time_zone >= timlthis.TimeBegin)
							)){

				now = stTime.tv_sec + timlthis.Time_zone;
				tm_now = gmtime(&now);

				if(Time_Day == 0){
					Time_Day = tm_now->tm_mday;
					memset(logSaveTxt,'\0',100);
					sprintf(logSaveTxt,"[RESTART]\n[TIMEBEGIN]%d\n",stTime.tv_sec);
					putLog(logSave,logSaveTxt);
				}

				if(Time_Day != tm_now->tm_mday){
					memset(fileSave,'\0',100);
					sprintf(fileSave,"%stime_lapse/%s",Path,TIME_LAPSE_SETUP_FILE);

					if(timl.TimeCha > 1)
					{
						timl.TimeCha = timl.TimeCha -1;
					}

					memset(tmpDay,'\0',100);
					sprintf(tmpDay,"%d",timl.TimeCha);
					write_profile_string("setup","TimeCha",tmpDay,fileSave);

					memset(logSaveTxt,'\0',100);
					sprintf(logSaveTxt,"[DAYCHA]%d\n",timl.TimeCha);
					putLog(logSave,logSaveTxt);
				}

				tim_now_date = tm_now->tm_hour * 60 + tm_now->tm_min;

				for(i=0;i<timlthis.Task_Num;i++){

					if((tim_now_date >= timlthis.Task_begin[i])&&(tim_now_date <= timlthis.Task_end[i])){

						GapTmp = timlthis.TimeGap;

//						if(pirContinuingStatus == 1){
//							if(timlthis.Time_Pir_Gap > 0){
//								if(timlthis.Time_Pir_Gap != timlthis.TimeGap){
//									GapTmp = timlthis.Time_Pir_Gap;
//								}
//							}
//						}


						if(stTime.tv_sec - stTimeOld.tv_sec >= GapTmp){
							gettimeofday(&stTimeOld, NULL);
							//拍照

							memset(tim_file_name,'\0',100);
							sprintf(tim_file_name,"%stime_lapse/time_Task_%d/%04d%02d%02d_%02d%02d%02d.jpg",Path,TL_Name,tm_now->tm_year+1900,tm_now->tm_mon+1, tm_now->tm_mday
									, tm_now->tm_hour, tm_now->tm_min, tm_now->tm_sec);

							Time_lapse_Save_Image(tim_file_name);

							memset(tim_file_name,'\0',100);
							sprintf(tim_file_name,"%stime_lapse/time_Task_%d/record.h264",Path,TL_Name);

							Time_lapse_Save_H264(tim_file_name);
						}

					}
				}
			}
			else if((timlthis.TimeEnd != 0)&&(stTime.tv_sec+timlthis.Time_zone > timlthis.TimeEnd)){
				//关闭任务

				memset(logSaveTxt,'\0',100);
				sprintf(logSaveTxt,"[End]%d\n",timlthis.TimeEnd);
				putLog(logSave,logSaveTxt);

				timlthis.Time_lapse_Num = 0;
				timlthis.Task_Num = 0;
				timlthis.TimeBegin = 0;
				timlthis.TimeEnd = 0;
				timlthis.TimeCha = 0;
				timlthis.TimeGap = 0;
				timlthis.Time_Pir_Gap = 0;

				Time_lapse_Set_Func(&timlthis,Path);
			}
		}

		if(time_line_Open == 1){
			if(time_line_ge > 0){
				if(stTime.tv_sec - stTimeOld.tv_sec >= time_line_ge){
					gettimeofday(&stTimeOld, NULL);

					now = stTime.tv_sec + get_timezone();
					tm_now = gmtime(&now);

					memset(tim_file_name,'\0',100);
					sprintf(tim_file_name,"%stime_lapse",Path);
					fileDir = opendir(tim_file_name);
					if(fileDir == NULL){
						mkdir(tim_file_name,0755);
					}else{
						closedir(fileDir);
					}

					memset(tim_file_name,'\0',100);
					sprintf(tim_file_name,"%stime_lapse/%04d%02d%02d_%02d%02d%02d.jpg",Path,tm_now->tm_year+1900,tm_now->tm_mon+1, tm_now->tm_mday
							, tm_now->tm_hour, tm_now->tm_min, tm_now->tm_sec);

					Time_lapse_Save_Image(tim_file_name);

					memset(tim_file_name,'\0',100);
					sprintf(tim_file_name,"%stime_lapse/record.h264",Path);

					Time_lapse_Save_H264(tim_file_name);

				}
			}
		}

	}else{
		if(stTime.tv_sec - stTimeOld.tv_sec > 300){
			//当前时间改为录像开始时间 TimeBegin + TimeCha

			stTimeOld.tv_sec = timlthis.TimeEnd + timlthis.TimeCha * 60 * 60 * 24;
			stTimeOld.tv_usec = 0;

			memset(logSaveTxt,'\0',100);
			sprintf(logSaveTxt,"[Restart]Time_Calibration faile\n[TIMEBEGIN]%d\n",stTimeOld.tv_sec);
			putLog(logSave,logSaveTxt);

			settimeofday(&stTimeOld, NULL );

		}else if(stTime.tv_sec == stTimeOld.tv_sec == 0){

			stTimeOld.tv_sec = 1400000000;
			stTimeOld.tv_usec = 0;

			memset(logSaveTxt,'\0',100);
			sprintf(logSaveTxt,"[Restart]Time_Calibration faile\n[TIMEBEGIN]%d\n",stTimeOld.tv_sec);
			putLog(logSave,logSaveTxt);

			settimeofday(&stTimeOld, NULL );
		}
	}
}



