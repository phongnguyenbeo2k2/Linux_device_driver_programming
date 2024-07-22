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
    gpiodev_drv_data_t *gpio_drv_data;
    /*This is macro function which point to available node (node have status is "okey")*/
    for_each_available_child_of_node(parent, child)
    {
        gpio_drv_data = devm_kzalloc(dev, sizeof(gpiodev_drv_data_t), GFP_KERNEL);
        if (gpio_drv_data == NULL)
        {
            pr_err("Error occur: when can not allocate memory for gpio_drv_data variable.\n");
            return -ENOMEM;
        }
        ret = of_property_read_string(child, "label", &label_name);
        if (ret < 0)
        {
            dev_warn(dev, "Don't get label information of gpio.\n");
            snprintf(gpio_drv_data->label, sizeof(gpio_drv_data->label), "unknow-gpio%d",i);
        }else 
        {
            strcpy(gpio_drv_data->label, label_name);
            dev_info(dev, "GPIO label = %s.\n",gpio_drv_data->label);
        }
    }
    return 0;
}
int gpio_sysfs_remove (struct platform_device *pdev)
{
    return 0;
}
/*Register the init and exit function with kernel*/
module_init(gpio_sysfs_init);
module_exit(gpio_sysfs_exit);

/*This is descriptive information about the module*/
MODULE_LICENSE("GPL");
MODULE_AUTHOR(AUTHOR);