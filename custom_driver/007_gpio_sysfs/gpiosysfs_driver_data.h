/*Define struct variable for platform driver*/
#ifndef GPIOSYSFS_DRIVER_DATA
#define GPIOSYSFS_DRIVER_DATA

#include <linux/module.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/cdev.h>
#include <linux/platform_device.h>
#include <linux/kdev_t.h>
#include <linux/uaccess.h>
#include <linux/slab.h>
#include <linux/mod_devicetable.h>
#include <linux/of.h>
#include <linux/of_device.h>

typedef struct 
{
    char label[20];
}gpiodev_drv_data_t;

typedef struct
{
    int total_device;
    struct class *gpio_sysfs_class;
}driver_data_t;

#endif