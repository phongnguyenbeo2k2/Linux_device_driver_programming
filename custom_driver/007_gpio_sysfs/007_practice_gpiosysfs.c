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

struct platform_driver gpio_sysfs_platform_driver = 
{
    .probe = gpio_sysfs_probe,
    .remove = gpio_sysfs_remove,,
    .driver =
    {
        .name = "gpio_sysfs_driver"
    }
};

driver_data_t gpio_drv_data;

/*begin detail function*/
static int __init gpio_sysfs_init(void)
{
    pr_info("Hello gpio sysfs driver.\n");
    return 0;
}
static void __exit gpio_sysfs_exit(void)
{
    pr_info("Goodbye gpio sysfs driver.\n");    
}
/*Register the init and exit function with kernel*/
module_init(gpio_sysfs_init);
module_exit(gpio_sysfs_exit);

/*This is descriptive information about the module*/
MODULE_LICENSE("GPL");
MODULE_AUTHOR(AUTHOR);