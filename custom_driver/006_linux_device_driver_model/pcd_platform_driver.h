#ifndef PCD_PLATFORM_DRIVER_H
#define PCD_PLATFORM_DRIVER_H

#include <linux/module.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/kdev_t.h>
#include <linux/uaccess.h>
#include "platform.h"
#include <linux/platform_device.h>
#include <linux/slab.h>
#include <linux/mod_devicetable.h>

#define AUTHOR "phong_foolish-gmail:phong.nguyenbeo2k2@hcmut.edu.vn"
#define MAX_DEVICE 10

#undef pr_fmt
#define pr_fmt(fmt) "%s :"fmt,__func__

/*declare prototype of function*/
int check_permission(int dev_perm, int access_mode);
int pcd_open (struct inode *p_inode, struct file *filp);
ssize_t pcd_write (struct file *filp, const char __user *user_buff, size_t count, loff_t *f_pos);
loff_t pcd_llseek (struct file *filp, loff_t off, int whence);
ssize_t pcd_read (struct file *filp, char __user *user_buff, size_t count, loff_t *f_pos);

/*driver private data structure*/
struct pcdrv_private_data
{
    /*The variable will allocate dynamically when device is detected*/
	int total_devices;
	/*This is hold device number of pseduo char device driver*/
	dev_t device_number_base; /*hold base address of devices*/
	struct class *pcd_class;
	struct device *pcd_device;
}; 

struct pcdev_private_data
{
    struct private_device_data pdata;
    dev_t device_number;
    char *buffer;
    struct cdev pcd_cdev;
    
};
struct device_data
{
    int property_1;
    int property_2;
};

enum pcdev_name
{
    DEVICEA1x,
    DEVICEB1x
};

#endif