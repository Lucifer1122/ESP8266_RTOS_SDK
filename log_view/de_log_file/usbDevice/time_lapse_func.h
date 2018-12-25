/*
 * time_lapse_func.h
 *
 *  Created on: 2015-10-28
 *      Author: guozhixin2
 */

#ifndef TIME_LAPSE_FUNC_H_
#define TIME_LAPSE_FUNC_H_

struct Time_lapse_struct
{
	int Time_lapse_Num;
	char Task_Num;
	short Task_begin[10];
	short Task_end[10];
	unsigned int TimeBegin;
	unsigned int TimeEnd;
	unsigned int TimeGap;
    int Time_zone;
	int TimeCha;
	int Time_Pir_Gap;
};

extern volatile struct Time_lapse_struct timl;

void Save_image_in_Path(char *Path);

void Time_lapse_Init_Func(char *Path);

int Time_lapse_Set_Func(struct Time_lapse_struct *timltmp,char *Path);

int Time_lapse_Get_Func(struct Time_lapse_struct *timltmp);

#endif /* TIME_LAPSE_FUNC_H_ */
