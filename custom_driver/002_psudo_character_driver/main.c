#include <linux/module.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/kdev_t.h>
#include <linux/uaccess.h>

/* This is a device of exercise*/
#define DEV_MEM_SIZE 512
char device_buffer[DEV_MEM_SIZE];
/*pseudo char device driver */

/*This is hold device number of pseduo char device driver*/
dev_t device_number;
/* Struc cdev for my device */
struct cdev pcd_cdev;

/*describe system call of pcd char device driver*/
loff_t pcd_llseek (struct file *filp, loff_t off, int whence)
{
	loff_t temp;
	pr_info("llseek requested \n");
	pr_info ("current value of the file position is %lld\n",filp->f_pos);
	switch (whence)
	{
		case SEEK_SET:
			if ((off > DEV_MEM_SIZE) || (off < 0))
			{
				return EINVAL;
			}
			filp->f_pos = off;
			break;
		case SEEK_CUR:
			temp = filp->f_pos + off;

			if ((temp > DEV_MEM_SIZE) || (temp < 0))
			{
				return EINVAL;
			}
			filp->f_pos = temp;
			break;
		case SEEK_END:

			temp = DEV_MEM_SIZE + off;
			if ((temp > DEV_MEM_SIZE) || (temp < 0))
			{
				return EINVAL;
			}
			filp->f_pos = temp;
			break;
		default:
			return EINVAL;
	}
	pr_info("The new value of the file position is %lld\n",filp->f_pos);
	return filp->f_pos;
}
ssize_t pcd_read (struct file *filp, char __user *user_buff, size_t count, loff_t *f_pos)
{
	pr_info ("read requested %zu bytes\n",count);
	pr_info ("current file positin is %lld\n", *f_pos);

	/*check counter bytes want to read*/
	if ((*f_pos + count) > DEV_MEM_SIZE)
	{
		count = DEV_MEM_SIZE - *f_pos;
	}
	/*copy kernel buffer to user buffer*/
	if (copy_to_user(user_buff,&device_buffer[*f_pos], count))
	{
		return -EFAULT;
	}
	/*fix file position*/
	*f_pos += count;

	pr_info ("The number of bytes is read: %zu bytes \n",count);
	pr_info ("current file position is %lld\n",*f_pos);
	/*retun the numbe of bytes that read successfully*/
	return count;

}
ssize_t pcd_write (struct file *filp, const char __user *user_buff, size_t count, loff_t *f_pos)
{
	pr_info("write request that it need to write %zu bytes!\n",count);
	pr_info ("current file positin is %lld\n", *f_pos);

	/*check counter bytes want to read*/
	if ((*f_pos + count) > DEV_MEM_SIZE)
	{
		count = DEV_MEM_SIZE - *f_pos;
	}
	/*Because count = 0 that is not enough memory to write data */
	if (!(count)) 
	{
		pr_err("No space left on my device!\n");
		return ENOMEM;
	}
	/*copy kernel buffer to user buffer*/
	if (copy_from_user(&device_buffer[*f_pos], user_buff, count))
	{
		return -EFAULT;
	}
	/*fix file position*/
	*f_pos += count;

	pr_info ("The number of bytes is wrote: %zu bytes \n",count);
	pr_info ("current file position is %lld\n",*f_pos);
	/*retun the numbe of bytes that write successfully*/
	return count;


	return 0;
}
int pcd_open (struct inode *p_inode, struct file *filp)
{
	pr_info("open requested!\n");
	return 0;
}
int pcd_release (struct inode *p_inode, struct file *filp)
{	
	pr_info ("release requested!\n");
	return 0;
}

/* File operations of my device driver */
struct file_operations pcd_fops = 
{
	.llseek = pcd_llseek,
	.read = pcd_read,
	.write = pcd_write,
	.open = pcd_open,
	.owner = THIS_MODULE
};

struct class *pcd_class;

struct device *pcd_device;
#undef pr_fmt  /*because pr_fmt have defined in header file linux/printk.h so that i need to undefine it*/
#define pr_fmt(fmt) "%s :"fmt,__func__ /*To show the line that is printed of what is function */

static int __init pseudo_driver_init(void)
{	
	int ret;
	/* 1. dynamically allocate a device number for my device driver*/
	ret = alloc_chrdev_region(&device_number,0,1,"pcd");
	if (ret < 0)
	{
		pr_err("Failure to init device number!\n");
		goto out;
	}
	/*MAJOR and MINOR marco in header file <linux/kdev_t>*/
	pr_info ("major number is: %d, minor number is: %d ",MAJOR(device_number), MINOR(device_number)); 
	/* 2. Initialize cdev structure with file operations pcd_fops*/
	cdev_init(&pcd_cdev, &pcd_fops);
	/* 3. Register a device with VFS */
	pcd_cdev.owner = THIS_MODULE;
	ret = cdev_add(&pcd_cdev, device_number,1);
	if (ret < 0)
	{
		pr_err("Failure to init register cdev with VFS!\n");
		goto un_alloc_chr_region;
	}
	/* 4. create folder under /sys/class*/
	pcd_class = class_create(THIS_MODULE,"pcd_class");
	if (IS_ERR(pcd_class))
        {
                pr_err("class creation failed!\n");
                ret = PTR_ERR(pcd_class);
                goto cdev_del;
        }

	/* 5. populate sysfs with device information of pcd and create device file*/
	pcd_device = device_create(pcd_class, NULL, device_number,NULL, "pcd_device_file");
	if (IS_ERR(pcd_device))
	{
		pr_err("device file creation failed!\n");
		ret = PTR_ERR(pcd_device);
		goto class_del;
	}
	pr_info("Initialize pcd successfully");
	return 0;

class_del:
	 class_destroy(pcd_class);
cdev_del:
	cdev_del(&pcd_cdev);
un_alloc_chr_region:
	unregister_chrdev_region(device_number,1);
out:
	pr_err("The module is not inserted!\n");
	return ret;
}

static void __exit pseudo_driver_cleanup(void)
{
	device_destroy(pcd_class,device_number);
	class_destroy(pcd_class);
	cdev_del(&pcd_cdev);
	unregister_chrdev_region(device_number,1);
	pr_info("module unloaded successfully!\n");
}


module_init(pseudo_driver_init);
module_exit(pseudo_driver_cleanup);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Phong");
MODULE_DESCRIPTION ("A pseudo char device driver");
