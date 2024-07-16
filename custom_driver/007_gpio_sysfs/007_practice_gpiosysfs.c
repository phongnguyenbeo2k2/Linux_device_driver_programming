#include <linux/module.h>
#include <linux/fs.h>
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

/*format the sting is printed to console by prefix the name of function is called and the line*/
#undef pr_fmt
#define pr_fmt(fmt) "%s (line %d): "fmt,__func__,__LINE__ 

/*declare function prototype and struct variable*/
static int __init gpio_sysfs_init(void);
static void __exit gpio_sysfs_exit(void);

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