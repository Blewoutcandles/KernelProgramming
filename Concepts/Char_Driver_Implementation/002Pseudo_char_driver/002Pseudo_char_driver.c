/*GPL LICENSE 2.0*/
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/kdev_t.h>
#include <linux/uaccess.h>

#undef pr_fmt
#define pr_fmt(fmt) "%s: "fmt, __func__

#define DEV_MEM_SIZE 512

/* char device's memory */
char device_buffer[DEV_MEM_SIZE];

/* This holds the device number */
dev_t device_number;

/* Cdev variable */
struct cdev chrdev_cdev;

loff_t mychardev_llseek (struct file *filp, loff_t off, int whence);
ssize_t mychardev_read (struct file *filp, char __user *buffer, size_t count, loff_t *f_pos);
ssize_t mychardev_write (struct file *filp, const char __user *buffer, size_t count, loff_t *f_pos);
int mychardev_open (struct inode *inode, struct file *filp);
int mychardev_release (struct inode *inode, struct file *filp);

loff_t mychardev_llseek (struct file *filp, loff_t offset, int whence)
{
	loff_t temp = 0;
	pr_info("llseek requested\n");
	pr_info("Current file position : %lld\n", filp->f_pos);
	
	switch(whence)
	{
		case SEEK_SET:
			if( (offset > DEV_MEM_SIZE) || (offset < 0) ){
				return -EINVAL;
			}
			filp -> f_pos = offset;
			break;
			
		case SEEK_CUR:
			temp = filp -> f_pos + offset;
			if( (temp > DEV_MEM_SIZE) || (temp < 0) ){
				return -EINVAL;
			}
			filp -> f_pos = temp;
			break;
			
		case SEEK_END:
			temp = DEV_MEM_SIZE + offset;
			if( (offset > DEV_MEM_SIZE) || (offset < 0) ){
				return -EINVAL;
			}
			filp -> f_pos = temp;
			break;

		default:
			break;
	}
	pr_info("Updated file position : %lld\n", filp->f_pos);
	return filp->f_pos;
}
ssize_t mychardev_read (struct file *filp, char __user *buffer, size_t count, loff_t *f_pos)
{
	int copystatus = 0;
	pr_info("read requested for %zu bytes\n", count);
	pr_info("Current file position : %lld\n", *f_pos);
	/*1. Adjust the count*/
	if((*f_pos + count) > DEV_MEM_SIZE){
		count = DEV_MEM_SIZE - *f_pos;
	}
	/*2. Copy to user*/
	copystatus = copy_to_user(buffer, &device_buffer[*f_pos], count);
	
	if(copystatus){
		pr_info("Some bytes could not be copied\n");
		return -EFAULT;
	}
	/*1. Update the current file position*/
	*f_pos = *f_pos + count;
	
	pr_info("Number of bytes successfully read : %zu\n", count);
	pr_info("Updated file position : %lld\n", *f_pos);
	return count;
}
ssize_t mychardev_write (struct file *filp, const char __user *buffer, size_t count, loff_t *f_pos)
{
	int number_of_bytes_written = 0;
	pr_info("Write requested for %zu bytes\n", count);
	pr_info("File position before writing: %lld\n", *f_pos); 
	
	if((*f_pos + count) > DEV_MEM_SIZE){
		count = DEV_MEM_SIZE - *f_pos;
	}
	if(count == 0){
		pr_info("Buffer overflow\n");
		return -ENOMEM;
	}
	number_of_bytes_written = copy_from_user(&device_buffer[*f_pos], buffer, count);
	
	if(number_of_bytes_written){
		pr_info("Some issues is writing to device memory\n");
		goto out;
	}
	
	*f_pos = *f_pos + count;
	
	pr_info("Number of bytes successfully written %zu\n", count);
	pr_info("Updated file position: %lld", *f_pos);
	out:
	return count;
}
int mychardev_open (struct inode *inode, struct file *filp)
{
	pr_info("Open was successful\n");
	return 0;
}
int mychardev_release (struct inode *inode, struct file *filp)
{
	pr_info("Release was successful\n");
	return 0;
}

/* file operations of the driver */
struct file_operations chrdev_fops = {
	.open = mychardev_open,
	.read = mychardev_read,
	.write = mychardev_write,
	.release = mychardev_release,
	.llseek = mychardev_llseek,
	.owner = THIS_MODULE
};

struct class* class_mychardev;
struct device* device_mychardev;

static int __init char_driver_start(void)
{
	int ret;
	/*1. Dynamically allocate a device number */
	ret = alloc_chrdev_region(&device_number, 0, 1, "mychardevice_devno");
	if(ret < 0){
		pr_info("Device number allocation failed\n");
		goto out;
	}
	
	pr_info("Device number <major>:<minor> = %d:%d\n", MAJOR(device_number), MINOR(device_number)); 
	/*2. Initialize the cdev structure with fops and owner is set to current module*/
	cdev_init(&chrdev_cdev, &chrdev_fops);
	chrdev_cdev.owner = THIS_MODULE;
	
	/*3. Register a device (cdev structure) with VFS using below cdev_add*/
	ret = cdev_add(&chrdev_cdev, device_number, 1);
	if(ret < 0){
		goto unreg_chardev;
	}
	
	/*4 Create a device class under /sys/class */
	//class_mychardev = class_create(THIS_MODULE, "mychardevice_class"); not compatible right now
	class_mychardev = class_create("mychardevice_class");
	if(IS_ERR(class_mychardev)){
		ret = PTR_ERR(class_mychardev);
		pr_err("Class creation failed\n");
		goto dev_del;
	}
	
	/* Populate the sysfs with device information. Create a dev file under /sys/class/class_name(mychardevice_class)/ */
	device_mychardev = device_create(class_mychardev, NULL, device_number, NULL, "mychardev"); 
	if(IS_ERR(device_mychardev)){
		ret = PTR_ERR(device_mychardev);
		pr_err("Device Creation failed\n");
		goto class_del;
	}
	/*mychardev will appear in dev file under /sys/class/mychardevice_class/*/
	
	pr_info("Module init was successful\n");
	
	return 0;

class_del:
	pr_err("Class destruction\n");
	class_destroy(class_mychardev);
dev_del:
	pr_err("cdev destruction\n");
	cdev_del(&chrdev_cdev);
unreg_chardev:
	pr_err("unregister cdev\n");
	unregister_chrdev_region(device_number, 1);
out:
	pr_info("Module initialization failed\n");
	return ret;
}


static void __exit char_driver_exit(void)
{
	/* Resource cleanup should be done exactly in reverse order of init function */ 
	device_destroy(class_mychardev, device_number);
	class_destroy(class_mychardev);
	cdev_del(&chrdev_cdev);
	unregister_chrdev_region(device_number, 1);
	pr_info("Module exit was successful\n");
}

module_init(char_driver_start);
module_exit(char_driver_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("marzjukm7@gmail.com");
MODULE_DESCRIPTION("Simple char device implementation of mine");
MODULE_INFO(BOARD, "RASPBERRYPI4");
