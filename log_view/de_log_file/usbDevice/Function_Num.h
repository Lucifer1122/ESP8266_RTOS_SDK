/*
 * Function_Num.h
 *
 *  Created on: 2015-10-15
 *      Author: guozhixin2
 */

#ifndef FUNCTION_NUM_H_
#define FUNCTION_NUM_H_

extern volatile int time_line_Open;

extern volatile int time_line_ge;

extern void set_USB_Save_Log_Func(char usb_save);
extern void set_SD_Save_Log_Func(char sd_save);
extern char get_USB_Save_Log_Func();
extern char get_SD_Save_Log_Func();
extern int get_Time_line_num();
extern void set_Time_line_num(int lin_num);
extern void init_Save_Log_Num();

#endif /* FUNCTION_NUM_H_ */
