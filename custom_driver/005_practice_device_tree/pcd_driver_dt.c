#include <linux/module.h>
#include <linux/fs.h>
#include "p_driver_data.h"
#include <linux/device.h>
#include <linux/cdev.h>
#include <linux/platform_device.h>
#include <linux/kdev_t.h>
#include <linux/uaccess.h>
#include <linux/slab.h>
#include <linux/mod_devicetable.h>

/*define macro*/
#define AUTHOR          "phong_foolish - gmail:phongqh01@gmail.com"
#define MAX_DEVICE      10
/*format the sting is printed to console by prefix the name of function is called and the line*/
#undef pr_fmt
#define pr_fmt(fmt) "%s (line %d): "fmt,__func__,__LINE__ 

/*declare function prototype and struct variable*/
static int __init phong_driver_init(void);
static void __exit phong_driver_exit(void);
int phong_open (struct inode *p_inode, struct file *filp);
ssize_t phong_write (struct file *filp, const char __user *user_buff, size_t count, loff_t *f_pos);
loff_t phong_llseek (struct file *filp, loff_t off, int whence);
ssize_t phong_read (struct file *filp, char __user *user_buff, size_t count, loff_t *f_pos);
int phong_platform_driver_probe (struct platform_device *pdev);
int phong_platform_driver_remove(struct platform_device *pdev);
int phong_release (struct inode *p_inode, struct file *filp);

enum pcdev_name
{
    DEVICEA1x,
    DEVICEB1x
};

struct device_data
{
    int property_1;
    int property_2;
};

struct device_data pcdrv_device_data[] =
{
    [DEVICEA1x] = {.property_1 = 12, .property_2 = 9},
    [DEVICEB1x] = {.property_1 = 9, .property_2 = 2002}
};

driver_data_t drv_data;
const struct of_device_id dt_list_match[] = 
{
    {.compatible = "pcdev-A1x", .data = (void *)DEVICEA1x},
    {.compatible = "pcdev-B1x", .data = (void *)DEVICEB1x},
    { }/*Null termination*/
};
struct file_operations pcd_fops = 
{
	.llseek = phong_llseek,
	.read = phong_read,
	.write = phong_write,
	.open = phong_open,
    .release = phong_release,
	.owner = THIS_MODULE
};
struct platform_driver phong_platform_driver =
{
    .probe = phong_platform_driver_probe,
    .remove = phong_platform_driver_remove,
    .driver = 
    {
        .name = "phong_char_device",
        .of_match_table = dt_list_match
    }
};

/*begin detail function*/
static int __init phong_driver_init(void)
{
    /*-------List task in init function-------*/
    /* 1. Allocate the device number for device by using dynamically method
       2. Create folder under /sys/class/ for device
       3. Register the platform driver with kernel
    */ 

    /*1. Allocate the device number for device by using dynamically method*/
    int ret;
    ret = alloc_chrdev_region(&drv_data.device_number_base, 0, MAX_DEVICE, "pcd");
    if (ret < 0)
    {
        pr_err("Failure to init device number!\n");
        return ret;
    }
    /*2. Create folder under /sys/class/ for device*/
    drv_data.pcd_class = class_create(THIS_MODULE, "phong-device");
    if (IS_ERR(drv_data.pcd_class))
    {
        pr_err("class creation failed!\n");
        ret = PTR_ERR(drv_data.pcd_class);
        unregister_chrdev_region(drv_data.device_number_base, MAX_DEVICE);
        return ret;
    }
    /*3. Register the platform driver with kernel*/
    ret = platform_driver_register(&phong_platform_driver);
    if (ret < 0)
    {
        pr_err("Register platform driver is failed!\n");
        class_destroy(drv_data.pcd_class);
        unregister_chrdev_region(drv_data.device_number_base, MAX_DEVICE);
        return ret;
    }
    pr_info("Driver have inserted to kernel!\n");
    return 0;
}
static void __exit phong_driver_exit(void)
{
    pr_info("Driver have removed from kernel!\n");
    /*1. unregister platform driver */
    platform_driver_unregister(&phong_platform_driver);
    /*2. delete folder under /sys/class*/
    class_destroy(drv_data.pcd_class);
    /*3. delete device number of devices*/
    unregister_chrdev_region(drv_data.device_number_base, MAX_DEVICE);
}

int phong_open (struct inode *p_inode, struct file *filp)
{
    pr_info("Opening deivce is successful");
    return 0;
}
ssize_t phong_write (struct file *filp, const char __user *user_buff, size_t count, loff_t *f_pos)
{
    pr_info("The write function is called!\n");
    return 0;
}
ssize_t phong_read (struct file *filp, char __user *user_buff, size_t count, loff_t *f_pos)
{
    pr_info("The read function is called!\n");
    return 0;
}
loff_t phong_llseek (struct file *filp, loff_t off, int whence)
{
    pr_info("The llseek function is called!\n");
    return 0;
}
int phong_release (struct inode *p_inode, struct file *filp)
{
    pr_info("Closing device is successfully!\n");
    return 0;
}
int phong_platform_driver_probe (struct platform_device *pdev)
{
    pr_info("The device is matched with driver!\n");
    return 0;
}
int phong_platform_driver_remove(struct platform_device *pdev)
{
    pr_info("The device is removed from the system!\n");
    return 0;
}
/*Register the init and exit function with kernel*/
module_init(phong_driver_init);
module_exit(phong_driver_exit);

/*This is descriptive information about the module*/
MODULE_LICENSE("GPL");
MODULE_AUTHOR(AUTHOR);
MODULE_DESCRIPTION ("A pseudo char device driver which handling 4 devices by device tree method");