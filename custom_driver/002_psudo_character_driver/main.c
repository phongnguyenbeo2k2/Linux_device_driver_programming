#include <linux/module.h>
#include <linux/fs.h>
#include <linux/cdev.h>

#define DEV_MEM_SIZE 512
char device_buffer[DEV_MEM_SIZE];
/*pseudo char device driver */

/*This is hold device number of pseduo char device driver*/
dev_t device_number;
/* Struc cdev for my device */
struct cdev pcd_cdev;

/* File operations of my device driver */
struct file_operations pcd_fops;

static int __init pseudo_driver_init(void)
{
	/* 1. dynamically allocate a device number for my device driver*/
	alloc_chrdev_region(&device_number,0,1,"pcd");
	/* 2. Initialize cdev structure with file operations pcd_fops*/
	cdev_init(&pcd_cdev, &pcd_fops);

	/* 3. Register a device with VFS */
	pcd_cdev.owner = THIS_MODULE;
	cdev_add(&pcd_cdev, device_number,1);
	return 0;
}

static void __exit pseudo_driver_cleanup(void)
{

}

module_init(pseudo_driver_init);
module_exit(pseudo_driver_cleanup);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Phong");
MODULE_DESCRIPTION ("A pseudo char device driver");
