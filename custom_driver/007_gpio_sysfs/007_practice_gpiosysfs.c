#include "gpiosysfs_driver_data.h"

/*define macro*/
#define AUTHOR          "phong_foolish - gmail:phongqh01@gmail.com"

/*format the sting is printed to console by prefix the name of function is called and the line*/
#undef pr_fmt
#define pr_fmt(fmt)     "%s (line %d): "fmt,__func__,__LINE__ 


/*declare function prototype and struct variable*/
/*----------------function----------------------*/
static int __init gpio_sysfs_init(void);
static void __exit gpio_sysfs_exit(void);
int gpio_sysfs_probe (struct platform_device *pdev);
int gpio_sysfs_remove (struct platform_device *pdev);
ssize_t direction_show(struct device *dev, struct device_attribute *attr,char *buf);
ssize_t direction_store(struct device *dev, struct device_attribute *attr,const char *buf, size_t count);
ssize_t value_show(struct device *dev, struct device_attribute *attr,char *buf);
ssize_t value_store(struct device *dev, struct device_attribute *attr,const char *buf, size_t count);
ssize_t label_show(struct device *dev, struct device_attribute *attr,char *buf);
/*----------------variable----------------------*/
const struct of_device_id gpio_sysfs_match[] = 
{
    {.compatible = "org,bone-gpio-sysfs"},
    { }/*Null termination*/
};
struct platform_driver gpio_sysfs_platform_driver = 
{
    .probe = gpio_sysfs_probe,
    .remove = gpio_sysfs_remove,
    .driver =
    {
        .name = "gpio_sysfs_driver",
        .of_match_table = of_match_ptr(gpio_sysfs_match)
    }
};

driver_data_t gpio_drv_data;
/*attribute variable*/
static DEVICE_ATTR_RW(direction);
static DEVICE_ATTR_RW(value);
static DEVICE_ATTR_RO(label);

struct attribute *gpio_attr[] = 
{
    &dev_attr_direction.attr,
    &dev_attr_value.attr,
    &dev_attr_label.attr,
    NULL
};
struct attribute_group gpio_attr_grp =
{
    .attrs = gpio_attr,
};
const struct attribute_group *group_attr_list[] = 
{
    &gpio_attr_grp,
    NULL
};
/*begin detail function*/
static int __init gpio_sysfs_init(void)
{
    int ret;
    /*Create the class in the /sys/class*/
    gpio_drv_data.gpio_sysfs_class = class_create(THIS_MODULE, "bone_gpios");
    if (IS_ERR(gpio_drv_data.gpio_sysfs_class))
    {
        pr_err("Error occur: when create class in /sys/class.\n");
        return PTR_ERR(gpio_drv_data.gpio_sysfs_class);
    }
    /*Sign up platform driver with kernel*/
    ret = platform_driver_register(&gpio_sysfs_platform_driver);
    if (ret < 0)
    {
        pr_err("Error occur: when register platform driver with kernel.\n");
        return ret;
    }

    pr_info("Gpio sysfs module driver is installed.\n");
    return 0;
}
static void __exit gpio_sysfs_exit(void)
{
    /*Unregister platform driver with kernel*/
    platform_driver_unregister(&gpio_sysfs_platform_driver);
    /*Delete class is created on init function*/
    class_destroy(gpio_drv_data.gpio_sysfs_class);
    pr_info("Gpio sysfs module driver is uninstalled.\n");    
}
int gpio_sysfs_probe (struct platform_device *pdev)
{
    int i = 1;
    int ret;
    struct device *dev = &pdev->dev;
    /*That is parent device node. In this case, that is "bone_gpio_devs" node*/
    struct device_node *parent = pdev->dev.of_node;
    struct device_node *child = NULL;
    const char *label_name; 
    gpiodev_drv_data_t *gpio_dev_drv_data;
    /*Get the total of child node*/
    gpio_drv_data.total_device = of_get_child_count(parent);
    if (gpio_drv_data.total_device == 0)
    {
        pr_err("No device is found from device tree\n");
        return -EINVAL;
    }
    pr_info("Total devices found = %d.\n",gpio_drv_data.total_device);
    /*Create array of device for controlling device which created under /sys/class*/
    gpio_drv_data.dev_sys = devm_kzalloc(dev, sizeof(struct device) * gpio_drv_data.total_device, GFP_KERNEL);
    /*This is macro function which point to available node (node have status is "okey")*/
    for_each_available_child_of_node(parent, child)
    {
        gpio_dev_drv_data = devm_kzalloc(dev, sizeof(gpiodev_drv_data_t), GFP_KERNEL);
        if (gpio_dev_drv_data == NULL)
        {
            pr_err("Error occur: when can not allocate memory for gpio_drv_data variable.\n");
            return -ENOMEM;
        }
        ret = of_property_read_string(child, "label", &label_name);
        if (ret < 0)
        {
            dev_warn(dev, "Don't get label information of gpio.\n");
            snprintf(gpio_dev_drv_data->label, sizeof(gpio_dev_drv_data->label), "unknow-gpio%d",i);
        }else 
        {
            strcpy(gpio_dev_drv_data->label, label_name);
            dev_info(dev, "GPIO label = %s.\n",gpio_dev_drv_data->label);
        }
        /*Begin stage 2:*/
        /*request gpio with gpio subsystem of kernel*/
        /*Because the child node store information of led, not parent node. So we can't use gpiod_get function, we must use devm_fwnode_get_gpiod_from_child function*/
        gpio_dev_drv_data->desc = devm_fwnode_get_gpiod_from_child(dev, "bone", &child->fwnode,GPIOD_ASIS, gpio_dev_drv_data->label);
        if (IS_ERR( gpio_dev_drv_data->desc))
        {
            ret = PTR_ERR(gpio_dev_drv_data->desc);
            if (ret == -ENOENT)
            {
                pr_err("No GPIO has been provied from request of you");
            }
            return ret;
        }

        /*set direction for gpio pin*/
        ret = gpiod_direction_output(gpio_dev_drv_data->desc, OFF);
        if (ret < 0)
        {
            dev_err(dev, "Set direction for %s is failed!.\n", gpio_dev_drv_data->label);
            return ret;
        }
        /*Satge 3: create device file under /sys/class/bone_gpios amd create attribute for led. And not create device file under /dev folder*/
        gpio_drv_data.dev_sys[i-1] = device_create_with_groups(gpio_drv_data.gpio_sysfs_class,dev, 0, gpio_dev_drv_data,group_attr_list, gpio_dev_drv_data->label);
        if (IS_ERR(gpio_drv_data.dev_sys[i-1]))
        {
            pr_err("Error occur: when create device file under /sys/class.\n"); 
            return PTR_ERR(gpio_drv_data.dev_sys[i-1]);           
        }
        i++;
    }
    pr_info("The gpio device is matched!\n");
    return 0;
}
int gpio_sysfs_remove (struct platform_device *pdev)
{
    int i;
    pr_info("The gpio device is removed!\n");
    for (i = 0; i < gpio_drv_data.total_device; i++)
    {
        device_unregister(gpio_drv_data.dev_sys[i]);
    }
    return 0;
}
ssize_t direction_show(struct device *dev, struct device_attribute *attr,char *buf)
{
    pr_info("Direction show is called.\n");
    return 0;
}
ssize_t direction_store(struct device *dev, struct device_attribute *attr,const char *buf, size_t count)
{
    pr_info("Direction store is called.\n");
    return count;
}

ssize_t value_show(struct device *dev, struct device_attribute *attr,char *buf)
{
    pr_info("Value show is called.\n");
    return 0;
}
ssize_t value_store(struct device *dev, struct device_attribute *attr,const char *buf, size_t count)
{
    pr_info("Value store is called.\n");
    return count;
}
ssize_t label_show(struct device *dev, struct device_attribute *attr,char *buf)
{
    pr_info("Label show is called.\n");
    return 0;
}
/*Register the init and exit function with kernel*/
module_init(gpio_sysfs_init);
module_exit(gpio_sysfs_exit);

/*This is descriptive information about the module*/
MODULE_LICENSE("GPL");
MODULE_AUTHOR(AUTHOR);