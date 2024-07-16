#include "pcd_platform_driver.h"

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
    /*determine whence from user space*/
    loff_t temp;
    struct pcdev_private_data *p_data = (struct pcdev_private_data *)filp->private_data;
    int max_size = p_data->pdata.size;
    pr_info("current file position is %lld\n", filp->f_pos);
    switch (whence)
    {
    case SEEK_SET:
        if ((off > max_size) || (off < 0))
        {
            return -EINVAL;
        }
        filp->f_pos = off;
        break;
    case SEEK_CUR:
        temp = filp->f_pos + off;
        if ((temp > max_size) || (temp < 0))
        {
            return -EINVAL;
        }
        filp->f_pos = temp;
        break;
    case SEEK_END:
        temp = max_size +  off;
        if ((temp > max_size) || (temp < 0))
        {
            return -EINVAL;
        }
        filp->f_pos = temp;
        break;
    default:
        return -EINVAL;
    }
	pr_info("The new value of the file position is %lld\n",filp->f_pos);
	return filp->f_pos;

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
