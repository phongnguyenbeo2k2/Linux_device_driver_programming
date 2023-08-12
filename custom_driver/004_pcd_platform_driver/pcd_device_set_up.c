#include <linux/module.h>
#include <linux/platform_device.h>

#include "platform.h"

/*Task on lesson
1. Create 2 struct variable of 2 device and struct private data
2. Creat init and exit function*/

#define AUTHOR "phong_foolish-gmail:phong.nguyenbeo2k2@hcmut.edu.vn"

#undef pr_fmt
#define pr_fmt(fmt) "%s :"fmt,__func__
void pcd_release(struct device * dev)
{
    pr_info("Device is released\n");
}
struct private_device_data pcd_device_data[2] =
{
    [0] = {.perm = RDWR, .size = 512, .serial = "PCD00"},
    [1] = {.perm = RDWR, .size = 1024,.serial = "PCD01"}
};

struct platform_device pcd_device_1 =
{
    .name = "pseudo_char_device",
    .id = 0,
    .dev = 
    {
        .platform_data = &pcd_device_data[0],
        .release = pcd_release
    }
};

struct platform_device pcd_device_2 = 
{
    .name = "pseudo_char_device",
    .id = 1,
    .dev = 
    {
        .platform_data = &pcd_device_data[1],
        .release = pcd_release
    }
};

static int __init pcd_platform_device_init(void)
{
    //register platform device

    platform_device_register(&pcd_device_1);
    platform_device_register(&pcd_device_2);
    pr_info("The device set up is loaded\n");
    return 0;
}

static void __exit pcd_platform_device_exit(void)
{
    platform_device_unregister(&pcd_device_1);
    platform_device_unregister(&pcd_device_2);
    pr_info("The device set up is unloaded\n");
}

module_init(pcd_platform_device_init);
module_exit(pcd_platform_device_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR(AUTHOR);
MODULE_DESCRIPTION("platform device driver of pseudo char device");