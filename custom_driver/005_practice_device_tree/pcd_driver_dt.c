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
#include <linux/stdarg.h>
#include <linux/of.h>
#include <linux/of_device.h>

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
device_data_t* get_device_info_from_dt(struct device *dev);

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
/*That variable is used for managing variable within driver*/
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
/*the kernel is support device tree, that mean CONFIG_OF in the .config file is y. */
/*used of_match_ptr function macro for detecting that the kernel is support device tree. */

struct platform_driver phong_platform_driver =
{
    .probe = phong_platform_driver_probe,
    .remove = phong_platform_driver_remove,
    .driver = 
    {
        .name = "phong_char_device",
        .of_match_table = of_match_ptr(dt_list_match)
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

device_data_t* get_device_info_from_dt(struct device *dev)
{
    device_data_t *dev_data;
    int ret;
    struct device_node *dev_node = dev->of_node;
    if (dev_node == NULL)
    {
        pr_info("This probe didn't happen because of device tree node.\n");
        return NULL;
    }
    dev_data = devm_kzalloc(dev, sizeof(*dev_data), GFP_KERNEL);
    if (dev_data == NULL)
    {
        pr_info("Can't allocate memory.\n");
        return ERR_PTR(-ENOMEM);
    }
    /*read property org,device-serial-num*/
    ret = of_property_read_string(dev_node,"org,device-serial-num",&dev_data->serial_number);
    if (ret != 0)
    {
        pr_info("Missing serial number property.\n");
        return ERR_PTR(-EINVAL);
    }
    /*read property org,size*/
    ret = of_property_read_u32(dev_node, "org,size", &dev_data->size);
    if (ret != 0)
    {
        pr_info("Missing size property.\n");
        return ERR_PTR(-EINVAL);
    }
    /*read permission property*/
    ret = of_property_read_u32(dev_node, "org,perm",&dev_data->perm);
    if (ret !=0)
    {
        pr_info("Missing permission propterty.\n");
        return ERR_PTR(-EINVAL);
    }

    return dev_data;
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
    /*List task*/
    /*1. Check the whether the device is created by device tree.*/
    /*2. Get the info of device from device tree and initialize device.*/
    /*3. Print the info of device which is pick up from device node.*/
    /*4. Dynamically allocate for buff field on struct pcdev_private_data*/
    /*5. Get the device number.*/
    /*6. Do cdev init with file operations and register cdev with kernel.*/
    /*7. Create device file.*/

    /*Now let's start*/
    device_data_t *dev_data;
    struct device *dev = &pdev->dev;
    device_drv_data_t *dev_drv_data;
    int driver_data;
    const struct of_device_id *match;
    int ret;
    /*1. Check the whether the device is created by device tree*/
    /*match will always be NULL if LINUX doesnt support device tree i.e CONFIG_OF is off*/
    match = of_match_device(of_match_ptr(dt_list_match),dev);
    if(match == NULL)
    {
        pr_info("The device is matched by device tree. Because kernel don't support device tree!\n");
        return -ENODEV;
    }
    /*2. Get the info of device from device tree and initialize device.*/
    dev_data = get_device_info_from_dt(dev);
    if(IS_ERR(dev_data))
    {
        return PTR_ERR(dev_data);
    }
    driver_data = (long)match->data;
    /*Save the pcd_received_data for else function can use it*/
    dev_drv_data->pdata.size = dev_data->size;
    dev_drv_data->pdata.serial_number = dev_data->serial_number;
    dev_drv_data->pdata.perm = dev_data->perm;
    /*3. Print the info of device which is pick up from device node.*/
    pr_info ("size of detected device is %d\n",dev_drv_data->pdata.size);
    pr_info ("serial number of detected device is %s\n ",dev_drv_data->pdata.serial_number);
    pr_info ("permission of detected device is %d\n",dev_drv_data->pdata.perm);
    pr_info("Config item 1 is %d\n", pcdrv_device_data[driver_data].property_1);
    pr_info("Config item 2 is %d\n", pcdrv_device_data[driver_data].property_2);    
    dev_set_drvdata(dev, dev_drv_data);
    /*4. Dynamically allocate for buff field on struct device_drv_data*/
    dev_drv_data->buffer = devm_kzalloc(dev,dev_drv_data->pdata.size, GFP_KERNEL);
    if (!dev_drv_data->buffer)
    {
        pr_info("Can't allocate memory.\n");
        return -ENOMEM;
    }
    /*5. Get the device number.*/
    dev_drv_data->device_number = drv_data.device_number_base + drv_data.total_devices;
    /*6. Do cdev init with file operations and register cdev with kernel.*/
    cdev_init(&dev_drv_data->pcd_cdev, &pcd_fops);
    dev_drv_data->pcd_cdev.owner = THIS_MODULE;
    ret = cdev_add(&dev_drv_data->pcd_cdev, dev_drv_data->device_number, 1);
    if (ret < 0)
    {
        pr_err("cdev add failed.\n");
        return ret;
    }
    /*7. Create device file into the class.*/
    drv_data.pcd_device = device_create(drv_data.pcd_class, dev, dev_drv_data->device_number, NULL, "pcd_device_file-%d", drv_data.total_devices);
    if (IS_ERR(drv_data.pcd_device))
    {
        pr_err("Device file creation is failed!.\n");
        ret = PTR_ERR(drv_data.pcd_device);
        cdev_del(&dev_drv_data->pcd_cdev);
        return ret;
    }

    drv_data.total_devices++;
    pr_info("The device is matched with driver!\n");
    return 0;
}
int phong_platform_driver_remove(struct platform_device *pdev)
{
    device_drv_data_t *dev_drv_data;
    dev_drv_data = (device_drv_data_t *)dev_get_drvdata(&pdev->dev);
    /*1. Remove a device file that created by device_create API in the class directory.*/
    device_destroy(drv_data.pcd_class, dev_drv_data->device_number);
    /*2. remove cdev that registered with VFS of kernel*/
    cdev_del(&dev_drv_data->pcd_cdev);

    drv_data.total_devices--;
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