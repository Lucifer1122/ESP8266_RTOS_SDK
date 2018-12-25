/*
 * usbDevice.h
 *
 *  Created on: Dec 9, 2015
 *      Author: guozhixin2
 */

#ifndef PLATFORMSERVE_USBDEVICE_USBDEVICE_H_
#define PLATFORMSERVE_USBDEVICE_USBDEVICE_H_

int CreateUsbDeviceThread();

void read_dir(char* path,char *repth, char* depth);

#endif /* PLATFORMSERVE_USBDEVICE_USBDEVICE_H_ */
