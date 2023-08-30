#include <linux/module.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/kdev_t.h>
#include <linux/uaccess.h>
#include "platform.h"
#include <linux/platform_device.h>

#define AUTHOR "phong_foolish-gmail:phong.nguyenbeo2k2@hcmut.edu.vn"
#define MAX_DEVICE 10

#undef pr_fmt
#define pr_fmt(fmt) "%s :"fmt,__func__

/*driver private data structure*/
struct pcdrv_private_data
{
	int total_devices;
	/*This is hold device number of pseduo char device driver*/
	dev_t device_number_base; /*hold base address of devices*/
	struct class *pcd_class;
	struct device *pcd_device;
};

struct pcdrv_private_data pcdrv_data;

int check_permission(int dev_perm, int access_mode)
{
	if (dev_perm == RDWR)
		return 0;
	if (dev_perm == RDONLY && ((access_mode & FMODE_READ) && !(access_mode & FMODE_WRITE) ))
	{
		return 0;
	}
	if (dev_perm == WRONLY && ((access_mode & FMODE_WRITE) && !(access_mode & FMODE_READ)))
		return 0;
	return -EPERM;
}

int pcd_open (struct inode *p_inode, struct file *filp)
{
	return 0;
}

int pcd_release (struct inode *p_inode, struct file *filp)
{	
	return 0;
}

ssize_t pcd_write (struct file *filp, const char __user *user_buff, size_t count, loff_t *f_pos)
{
    return -ENOMEM;
}
loff_t pcd_llseek (struct file *filp, loff_t off, int whence)
{
    return 0;
}
ssize_t pcd_read (struct file *filp, char __user *user_buff, size_t count, loff_t *f_pos)
{
	return 0;
}


struct file_operations pcd_fops = 
{
	.llseek = pcd_llseek,
	.read = pcd_read,
	.write = pcd_write,
	.open = pcd_open,
	.owner = THIS_MODULE
};

int pcd_platform_driver_probe (struct platform_device *pdev)
{
    /* data */
    pr_info("The device is detected\n");
    return 0;
};

int pcd_platform_driver_remove(struct platform_device *pdev)
{
    /* data */
    pr_info("The device is removed\n");
    return 0;
};

struct platform_driver pcd_platform_driver = 
{
    .probe = pcd_platform_driver_probe,
    .remove = pcd_platform_driver_remove,
    .driver =
    {
        .name = "pseudo_char_device"
    }
};
static int __init pseudo_driver_init(void)
{
    /*1. allocate device number dynamically for device*/
    int ret;
    ret = alloc_chrdev_region(&pcdrv_data.device_number_base,0,MAX_DEVICE,"pcd");
	if (ret < 0)
	{
		pr_err("Failure to init device number!\n");
		return ret;
	}
    /*2. create folder under /sys/class*/
    pcdrv_data.pcd_class = class_create(THIS_MODULE,"pcd_class");
	if (IS_ERR(pcdrv_data.pcd_class))
        {
                pr_err("class creation failed!\n");
                ret = PTR_ERR(pcdrv_data.pcd_class);
                unregister_chrdev_region(pcdrv_data.device_number_base,MAX_DEVICE);
        }
    /*3. register platform driver*/
    ret = platform_driver_register(&pcd_platform_driver);
    if (ret < 0)
    {
        pr_err("Refister platform driver is failed");
        class_destroy(pcdrv_data.pcd_class);
        unregister_chrdev_region(pcdrv_data.device_number_base,MAX_DEVICE);
        return ret;
    }
    pr_info("The platform driver is added\n");
    return 0;
}

static void __exit pseudo_driver_cleanup(void)
{
    /*1. unregister platform driver */
    platform_driver_unregister(&pcd_platform_driver);
    /*2. delete folder under /sys/class*/
    class_destroy(pcdrv_data.pcd_class);
    /*3. delete device number of devices*/
    unregister_chrdev_region(pcdrv_data.device_number_base,MAX_DEVICE);
    pr_info("The platform driver is removed\n");
}

module_init(pseudo_driver_init);
module_exit(pseudo_driver_cleanup);

MODULE_LICENSE("GPL");
MODULE_AUTHOR(AUTHOR);
MODULE_DESCRIPTION ("A pseudo char device driver which handling 4 devices");