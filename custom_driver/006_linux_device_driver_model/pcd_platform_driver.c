#include "pcd_platform_driver.h"

/*declare prototype of show and store function of attribute*/
ssize_t max_size_show(struct device *dev, struct device_attribute *attr,char *buf);
ssize_t max_size_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
ssize_t serial_num_show(struct device *dev, struct device_attribute *attr,char *buf);
int create_attribute_file_sys(struct device *dev);
/*create 2 variable of struct device_attribute*/
static DEVICE_ATTR(max_size,S_IRUGO|S_IWUSR, max_size_show, max_size_store);
static DEVICE_ATTR(serial_num, S_IRUGO, serial_num_show, NULL);

struct attribute *pcd_attr[] = 
{
    &dev_attr_max_size.attr,
    &dev_attr_serial_num.attr,
    NULL
};

struct attribute_group pcd_attr_grp =
{
    .attrs = pcd_attr,
};
struct pcdrv_private_data pcdrv_data;

struct platform_device_id pcd_ids[] = 
{
    [0] = {.name = "device-A1x", .driver_data = DEVICEA1x},
    [1] = {.name = "device-B1x", .driver_data = DEVICEB1x},
    { }/*Null termination*/
};

struct device_data pcdrv_device_data[] =
{
    [DEVICEA1x] = {.property_1 = 12, .property_2 = 9},
    [DEVICEB1x] = {.property_1 = 9, .property_2 = 2002}
};


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
    /*When matching between driver and device is sucessfully, so the id_entry variable of platform device will assigned with 
    the id_table variable of platform driver. The detail operation in the function platform_match.
    */
    pr_info("property 1 of %s is %d\n",pdev->name, pcdrv_device_data[pdev->id_entry->driver_data].property_1);
    pr_info("property 2 of %s is %d\n",pdev->name, pcdrv_device_data[pdev->id_entry->driver_data].property_2);
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
	pcdrv_data.pcd_device = device_create(pcdrv_data.pcd_class, &pdev->dev, pcd_received_data->device_number,NULL, "pcd_device_file-%d",pdev->id+1);
	if (IS_ERR(pcdrv_data.pcd_device))
	{
		pr_err("device file creation failed!\n");
		ret = PTR_ERR(pcdrv_data.pcd_device);
		goto cdev_delete;
	}
    pcdrv_data.total_devices++; /*total device that driver control*/

    /*Create attribute in the sys file system*/
    ret = create_attribute_file_sys(pcdrv_data.pcd_device);
    if (ret < 0)
    {
        pr_err("Creating max_size and serial_num attribute is failed.\n");
        device_destroy(pcdrv_data.pcd_class, pcd_received_data->device_number);
        goto cdev_delete;
    }
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
    .id_table = pcd_ids,
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

ssize_t max_size_show(struct device *dev, struct device_attribute *attr,char *buf)
{    
    struct pcdev_private_data *pcd_received_data;
    pcd_received_data = (struct pcdev_private_data *)dev_get_drvdata(dev->parent);
    pr_info ("The max size show method is called.\n");
    return sprintf(buf, "%d\n",pcd_received_data->pdata.size);
}
ssize_t max_size_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    struct pcdev_private_data *pcd_received_data;
    long result;
    int ret;
    pcd_received_data = (struct pcdev_private_data *)dev_get_drvdata(dev->parent);
    ret = kstrtol(buf, 10, &result);
    if (ret < 0)
    {
        pr_err("Can't change the size of device!.\n");
        return ret;
    }
    pr_info ("The changed size is %ld.\n",result);
    pcd_received_data->pdata.size = (int)result;
    pcd_received_data->buffer = krealloc(pcd_received_data->buffer, pcd_received_data->pdata.size, GFP_KERNEL);
    pr_info("Max size store is called!.\n");
    return count;
}
ssize_t serial_num_show(struct device *dev, struct device_attribute *attr,char *buf)
{
    /*This struct device *dev that not is represented of device, it's device directory which created in the /sys/class/xxx*/
    struct pcdev_private_data *pcd_received_data;
    pcd_received_data = (struct pcdev_private_data *)dev_get_drvdata(dev->parent);
    pr_info("The serial num show method is called.\n");
    return sprintf(buf, "%s\n", pcd_received_data->pdata.serial_number);
}
int create_attribute_file_sys(struct device *dev)
{
#if 0
    int ret;
    ret = sysfs_create_file(&dev->kobj, &dev_attr_max_size.attr);
    if (ret < 0)
    {
        pr_err("Max size attribute is not created successfully.\n");
        return ret;
    }
    ret = sysfs_create_file(&dev->kobj, &dev_attr_serial_num.attr);
    if (ret < 0)
    {
        pr_err("Serial num attribute is not created suscessfully.\n");
        return ret;
    }

    return 0;
#endif
    /*Using attribute group for creating attribute in the sys file system*/
    return sysfs_create_group(&dev->kobj, &pcd_attr_grp);
}
module_init(pseudo_driver_init);
module_exit(pseudo_driver_cleanup);

MODULE_LICENSE("GPL");
MODULE_AUTHOR(AUTHOR);
MODULE_DESCRIPTION ("A pseudo char device driver which handling 4 devices");