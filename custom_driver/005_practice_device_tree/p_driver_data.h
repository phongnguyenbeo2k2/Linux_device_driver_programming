#ifndef DRIVER_DATA
#define DRIVER_DATA

#include <linux/module.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include "p_device_data.h"

/*This struct used to store data of driver*/
typedef struct 
{
    /*The variable will allocate dynamically when device is detected*/
	int total_devices;
	/*This is hold device number of pseduo char device driver*/
	dev_t device_number_base; /*hold base address of devices*/
	struct class *pcd_class;
	struct device *pcd_device;
}driver_data_t;

/*This struct used to store device info, which is only used within this driver*/
typedef struct 
{
    device_data_t pdata;
    dev_t device_number;
    char *buffer;
    struct cdev pcd_cdev;
}device_drv_data_t;

#endif