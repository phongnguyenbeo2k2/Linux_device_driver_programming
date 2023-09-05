#include <linux/module.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/kdev_t.h>
#include <linux/uaccess.h>
#include "platform.h"
#include <linux/platform_device.h>
#include <linux/slab.h>

#define AUTHOR "phong_foolish-gmail:phong.nguyenbeo2k2@hcmut.edu.vn"
#define MAX_DEVICE 10

#undef pr_fmt
#define pr_fmt(fmt) "%s :"fmt,__func__

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
    struct pcdev_private_data *p_data;
    int minor_number;
    int major_number;
    int ret;
    /*1. Check whether the device file that you open*/
    minor_number = MINOR(p_inode->i_rdev);
    major_number = MAJOR(p_inode->i_rdev);
    pr_info("major number: %d and minor number: %d",major_number, minor_number);
    /*2. get specification information of device use contain_of API*/
    p_data = container_of(p_inode->i_cdev, struct pcdev_private_data, pcd_cdev);
    /*3. Save these specification information of device for read, write method*/
    filp->private_data = p_data;
    /*4. Check permission whether it is correct with device*/
    ret = check_permission(p_data->pdata.perm, filp->f_mode);
    if (!ret)
	{
		pr_info("open is successful\n");
	}else {
		pr_info ("open is unsuccessful\n");
	}
	return ret;
}

int pcd_release (struct inode *p_inode, struct file *filp)
{	
	return 0;
}

ssize_t pcd_write (struct file *filp, const char __user *user_buff, size_t count, loff_t *f_pos)
{
    struct pcdev_private_data *p_data = (struct pcdev_private_data *)filp->private_data;
    int max_size = p_data->pdata.size;
	pr_info("write request that it need to write %zu bytes!\n",count);
	pr_info ("current file positin is %lld\n", *f_pos);
    /*1. check whether the number of bytes is written from user that exceed max_size of device*/
    if ((*f_pos + count) > max_size)
    {
        count = max_size - *f_pos;
    }

    if (!(count))
    {
        pr_info("No space left in the device\n");
        return -ENOMEM;
    }
    /*2. write count bytes from user space to device memory*/
    if(copy_from_user(p_data->buffer + (*f_pos),user_buff,count))
    {
        pr_info("The process write down device that is failed\n");
        return -EFAULT;
    }
    /*3. update the f_pos*/
    *f_pos += count;
    /*4. return result of write method*/
    pr_info ("The number of bytes is wrote: %zu bytes \n",count);
	pr_info ("current file position is %lld\n",*f_pos);
	/*retun the numbe of bytes that write successfully*/
	return count;
}
loff_t pcd_llseek (struct file *filp, loff_t off, int whence)
{
    return 0;
}
ssize_t pcd_read (struct file *filp, char __user *user_buff, size_t count, loff_t *f_pos)
{
    struct pcdev_private_data *p_data = (struct pcdev_private_data *)filp->private_data;
    int max_size = p_data->pdata.size;
	pr_info("read request that it need to write %zu bytes!\n",count);
	pr_info ("current file position is %lld\n", *f_pos);
    /*1. check whether the number of bytes is written from user that exceed max_size of device*/
    if ((*f_pos + count) > max_size)
    {
        count = max_size - *f_pos;
    }
    /*2. write count bytes from user space to device memory*/
    if(copy_to_user(user_buff,p_data->buffer +(*f_pos),count))
    {
        pr_info("The process write down device that is failed\n");
        return -EFAULT;
    }
    /*3. update the f_pos*/
    *f_pos += count;
    /*4. return result of write method*/
    pr_info ("The number of bytes is reead: %zu bytes \n",count);
	pr_info ("current file position is %lld\n",*f_pos);
	/*retun the number of bytes that write successfully*/
	return count;   
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
    struct pcdev_private_data *pcd_received_data;
    struct private_device_data *p_data;
    int ret;
     pr_info("The device is detected\n");

    /*1. get platform data from platform_device
    Why do that ?
    Because the data that this function receive is able to incorrect. So that you should check it before use it*/
    p_data = (struct private_device_data *)dev_get_platdata(&pdev->dev); /*api dev_get_platdata help me get platform data easily*/
    if(!p_data)
    {
        pr_info ("The received platform data isn't available\n");
        ret = -EINVAL;
        goto out;
    }
    pcd_received_data = devm_kzalloc(&pdev->dev,sizeof(*pcd_received_data),GFP_KERNEL);
    if (!pcd_received_data)
    {
        pr_info("Don't allocate memory");
        ret = -ENOMEM;
        goto out;
    }
    /*Save the pcd_received_data for else function can use it*/
    dev_set_drvdata(&pdev->dev,pcd_received_data);
    /*2.get data from received platform data*/
    pcd_received_data->pdata.size = p_data->size;
    pcd_received_data->pdata.serial_number = p_data->serial_number;
    pcd_received_data->pdata.perm = p_data->perm;

    pr_info ("size of detected device is %d\n",pcd_received_data->pdata.size);
    pr_info ("serial number of detected device is %s\n ",pcd_received_data->pdata.serial_number);
    pr_info ("permission of detected device is %d\n",pcd_received_data->pdata.perm);
   
    /*3. Dynamically allocate for buff field on struct pcdev_private_data*/
    pcd_received_data->buffer = devm_kzalloc(&pdev->dev,pcd_received_data->pdata.size,GFP_KERNEL);
    if (!(pcd_received_data->buffer))
    {
        /*The buffer is memory of device*/
        pr_info("Don't allocate memory for buffer\n"); 
        ret = -ENOMEM;
        goto pcd_received_data_free;
    }
    /*4. Get the device number*/
    pcd_received_data->device_number = pcdrv_data.device_number_base + pdev->id;
    /*5. Do cdev init and cdev add*/
    cdev_init(&pcd_received_data->pcd_cdev,&pcd_fops);
    pcd_received_data->pcd_cdev.owner = THIS_MODULE;
    ret = cdev_add(&pcd_received_data->pcd_cdev, pcd_received_data->device_number,1);
	if (ret < 0)
	{
		pr_err("Failure to init register cdev with VFS!\n");
		goto free_buffer_of_device;
	}
    /*6. Create device file for the device is detected*/
	pcdrv_data.pcd_device = device_create(pcdrv_data.pcd_class, NULL, pcd_received_data->device_number,NULL, "pcd_device_file-%d",pdev->id+1);
	if (IS_ERR(pcdrv_data.pcd_device))
	{
		pr_err("device file creation failed!\n");
		ret = PTR_ERR(pcdrv_data.pcd_device);
		goto cdev_delete;
	}
    pcdrv_data.total_devices++; /*total device that driver control*/
    pr_info("The probe function was successful\n");
    return 0;

    /*7. Error handling*/
cdev_delete:
    cdev_del(&pcd_received_data->pcd_cdev);
free_buffer_of_device:
    devm_kfree(&pdev->dev,pcd_received_data->buffer);
pcd_received_data_free:
    devm_kfree(&pdev->dev,pcd_received_data);
out:
    pr_info("The probe is failed\n");
    return ret;
};

int pcd_platform_driver_remove(struct platform_device *pdev)
{
    /* data */
    struct pcdev_private_data *pcd_received_data;
    pcd_received_data = (struct pcdev_private_data *)dev_get_drvdata(&pdev->dev);
    /*1. Remove a device that created by device_create API*/
    device_destroy(pcdrv_data.pcd_class, pcd_received_data->device_number);
    /*2. remove cdev that registered with VFS of kernel*/
    cdev_del(&pcd_received_data->pcd_cdev);
    // /*3. free all memory is own device*/ that is not need because when use devm_kmalloc so the kernel take care of freeing memory 
    //   when device or driver is removed from system 
    // kfree(pcd_received_data->buffer);
    // kfree(pcd_received_data);

    pcdrv_data.total_devices--;
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
                return ret;
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