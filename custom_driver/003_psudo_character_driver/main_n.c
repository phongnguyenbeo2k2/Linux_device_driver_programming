#include <linux/module.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/kdev_t.h>
#include <linux/uaccess.h>

#define NO_OF_DEVICE 4

/*size memory of 4 devices */
#define MEM_SIZE_DEV1 1024
#define MEM_SIZE_DEV2 512
#define MEM_SIZE_DEV3 1024
#define MEM_SIZE_DEV4 512

/*memory for 4 devices*/
char device_1_buffer[MEM_SIZE_DEV1];
char device_2_buffer[MEM_SIZE_DEV2];
char device_3_buffer[MEM_SIZE_DEV3];
char device_4_buffer[MEM_SIZE_DEV4];

/*device's private data*/
struct pcdev_private_data
{
	char *buffer;
	unsigned size;
	const char *serial_number;
	int perm;
/* Struc cdev for my device */
	struct cdev cdev;	
};

/*driver private data structure*/
struct pcdrv_private_data
{
	int total_devices;
	/*This is hold device number of pseduo char device driver*/
	dev_t device_number;
	struct class *pcd_class;
	struct device *pcd_device;
	struct pcdev_private_data pcdev_data[NO_OF_DEVICE];
};

#define RDONLY 0x01
#define WRONLY 0x10
#define RDWR   0x11
struct pcdrv_private_data pcdrv_data =
{
	.total_devices = NO_OF_DEVICE,
	.pcdev_data = 
	{
			[0] = 
			{
				.buffer = device_1_buffer,
				.size = MEM_SIZE_DEV1,
				.serial_number = "PCDEV1A000",
				.perm = RDONLY /*RDONLY permission*/
			},
			[1] = 
			{
				.buffer = device_2_buffer,
				.size = MEM_SIZE_DEV2,
				.serial_number = "PCDEV2A001",
				.perm = WRONLY /*WRONLY permission*/
			},
			[2] = 
			{
				.buffer = device_3_buffer,
				.size = MEM_SIZE_DEV3,
				.serial_number = "PCDEV2A002",
				.perm = RDWR /*RDWR permission*/
			},
			[3] = 
			{
				.buffer = device_4_buffer,
				.size = MEM_SIZE_DEV4,
				.serial_number = "PCDEV3A003",
				.perm = RDWR /*RDWR permission*/
			},
	}
};

/*pseudo char device driver */


/*describe system call of pcd char device driver*/
loff_t pcd_llseek (struct file *filp, loff_t off, int whence)
{
	loff_t temp;
	struct pcdev_private_data *pcdev_data = (struct pcdev_private_data*)filp->private_data;
	int max_size = pcdev_data->size;
	pr_info("llseek requested \n");
	pr_info ("current value of the file position is %lld\n",filp->f_pos);
	switch (whence)
	{
		case SEEK_SET:
			if ((off > max_size) || (off < 0))
			{
				return EINVAL;
			}
			filp->f_pos = off;
			break;
		case SEEK_CUR:
			temp = filp->f_pos + off;

			if ((temp > max_size) || (temp < 0))
			{
				return EINVAL;
			}
			filp->f_pos = temp;
			break;
		case SEEK_END:

			temp = max_size + off;
			if ((temp > max_size) || (temp < 0))
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
	struct pcdev_private_data *pcdev_data = (struct pcdev_private_data*)filp->private_data;
	int max_size = pcdev_data->size;
	pr_info ("read requested %zu bytes\n",count);
	pr_info ("current file positin is %lld\n", *f_pos);
	/*check counter bytes want to read*/
	if ((*f_pos + count) > max_size)
	{
		count = max_size - *f_pos;
	}
	/*copy kernel buffer to user buffer*/
	if (copy_to_user(user_buff,pcdev_data->buffer + (*f_pos), count))
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
	struct pcdev_private_data *pcdev_data = (struct pcdev_private_data*)filp->private_data;
	int  max_size;
	max_size = pcdev_data->size;
	pr_info("write request that it need to write %zu bytes!\n",count);
	pr_info ("current file positin is %lld\n", *f_pos);

	/*check counter bytes want to read*/
	if ((*f_pos + count) > max_size)
	{
		count = max_size - *f_pos;
	}
	if (!(count))
	{
		pr_err("No space left on my device!\n");
		return ENOMEM;
	}
	/*copy kernel buffer to user buffer*/
	if (copy_from_user(pcdev_data->buffer + (*f_pos), user_buff, count))
	{
		return -EFAULT;
	}
	/*fix file position*/
	*f_pos += count;

	pr_info ("The number of bytes is wrote: %zu bytes \n",count);
	pr_info ("current file position is %lld\n",*f_pos);
	/*retun the numbe of bytes that write successfully*/
	return count;

}

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
	int ret;

	int minor_number;

	struct pcdev_private_data *pcdev_data;

	/*check which device file is opened by user space*/
	minor_number = MINOR(p_inode->i_rdev);
	pr_info ("minor acess = %d\n",minor_number);

	/*get device's private data structures from device*/
	pcdev_data = container_of(p_inode->i_cdev, struct pcdev_private_data, cdev);

	/*Because other method need also device private data so you have to provide it to other methods of driver*/
	filp->private_data = pcdev_data;
	/*check permission of device*/
	ret = check_permission(pcdev_data->perm, filp->f_mode);

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


#undef pr_fmt
#define pr_fmt(fmt) "%s :"fmt,__func__

static int __init pseudo_driver_init(void)
{	

	int ret;
	int i;
	/* 1. dynamically allocate a device number for my device driver*/
	ret = alloc_chrdev_region(&pcdrv_data.device_number,0,NO_OF_DEVICE,"pcd");
	if (ret < 0)
	{
		pr_err("Failure to init device number!\n");
		goto out;
	}
	/* create folder under /sys/class*/
	pcdrv_data.pcd_class = class_create(THIS_MODULE,"pcd_class");
	if (IS_ERR(pcdrv_data.pcd_class))
        {
                pr_err("class creation failed!\n");
                ret = PTR_ERR(pcdrv_data.pcd_class);
                goto un_alloc_chr_region;
        }
	for (i = 0; i < NO_OF_DEVICE; i++)
	{
		pr_info ("major number is: %d, minor number is: %d ",MAJOR(pcdrv_data.device_number + i), MINOR(pcdrv_data.device_number + i));
		/* Initialize cdev structure with file operations pcd_fops*/
		 cdev_init(&pcdrv_data.pcdev_data[i].cdev, &pcd_fops);
		/* Register a device with VFS */
		pcdrv_data.pcdev_data[i].cdev.owner = THIS_MODULE;
		ret = cdev_add(&pcdrv_data.pcdev_data[i].cdev, pcdrv_data.device_number+i,1);
		if (ret < 0)
		{
			pr_err("Failure to init register cdev with VFS!\n");
			goto cdev_del;
		}
		/* populate sysfs with device information of pcd and create device file*/
		pcdrv_data.pcd_device = device_create(pcdrv_data.pcd_class, NULL, pcdrv_data.device_number+i,NULL, "pcd_device_file-%d",i+1);
		if (IS_ERR(pcdrv_data.pcd_device))
		{
			pr_err("device file creation failed!\n");
			ret = PTR_ERR(pcdrv_data.pcd_device);
			goto class_del;
		}
	}

	pr_info("Initialize pcd successfully");
	return 0;

cdev_del:
class_del:
	for (; i >= 0; i--)
	{
		device_destroy(pcdrv_data.pcd_class, pcdrv_data.device_number+i);
		cdev_del(&pcdrv_data.pcdev_data[i].cdev);
	}
	class_destroy(pcdrv_data.pcd_class);
un_alloc_chr_region:
	unregister_chrdev_region(pcdrv_data.device_number,NO_OF_DEVICE);
out:
	pr_err("The module is not inserted!\n");
	return ret;
	return 0;
}

static void __exit pseudo_driver_cleanup(void)
{
	int i;
	for ( i = 0; i < NO_OF_DEVICE;i++)
	{
		device_destroy(pcdrv_data.pcd_class, pcdrv_data.device_number+i);
		cdev_del(&pcdrv_data.pcdev_data[i].cdev);
	}
	class_destroy(pcdrv_data.pcd_class);
	unregister_chrdev_region(pcdrv_data.device_number,NO_OF_DEVICE);
	pr_info("module unloaded successfully\n");
}


module_init(pseudo_driver_init);
module_exit(pseudo_driver_cleanup);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Phong");
MODULE_DESCRIPTION ("A pseudo char device driver which handling 4 devices");
