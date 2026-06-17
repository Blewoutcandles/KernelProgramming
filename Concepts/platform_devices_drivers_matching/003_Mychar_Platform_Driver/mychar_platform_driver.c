/*GPL LICENSE 2.0*/
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/kdev_t.h>
#include <linux/uaccess.h>
#include <linux/platform_device.h>
#include <linux/slab.h>
#include "platform.h"

#undef pr_fmt
#define pr_fmt(fmt) 		"%s: "fmt, __func__

#define DEV_MEM_SIZE 		512
#define NO_OF_DEVICES 		4
#define MEM_SIZE_MAX_PCDEV1	1024
#define MEM_SIZE_MAX_PCDEV2	1024
#define MEM_SIZE_MAX_PCDEV3	1024
#define MEM_SIZE_MAX_PCDEV4	1024


/* char device's memory */
char device_buffer[DEV_MEM_SIZE];
char device_buffer_pcdev1[MEM_SIZE_MAX_PCDEV1];
char device_buffer_pcdev2[MEM_SIZE_MAX_PCDEV2];
char device_buffer_pcdev3[MEM_SIZE_MAX_PCDEV3];
char device_buffer_pcdev4[MEM_SIZE_MAX_PCDEV4];

loff_t mychardev_llseek (struct file *filp, loff_t off, int whence);
ssize_t mychardev_read (struct file *filp, char __user *buffer, size_t count, loff_t *f_pos);
ssize_t mychardev_write (struct file *filp, const char __user *buffer, size_t count, loff_t *f_pos);
int mychardev_open (struct inode *inode, struct file *filp);
int mychardev_release (struct inode *inode, struct file *filp);
int mychar_platform_driver_probe(struct platform_device *mychardev);
int mychar_platform_driver_remove(struct platform_device *mychardev);

/* file operations of the driver */
struct file_operations dev_fops_mychr = {
	.open = mychardev_open,
	.read = mychardev_read,
	.write = mychardev_write,
	.release = mychardev_release,
	.llseek = mychardev_llseek,
	.owner = THIS_MODULE
};
/*Device private data structure*/
struct dev_private_data_mychr{
	struct dev_platform_data_mychr mychrdata;
    char* buffer;
    dev_t dev_num;	/*holds device number*/
    struct cdev cdev_chr;
};

/*Driver private data structure*/
struct drv_private_data_mychr{
	int total_devices;
  	dev_t device_num_base;
 	struct class* class_mychardev;
	struct device* device_mychardev;
};

/*Driver private data structure member initialization*/
struct drv_private_data_mychr drv_prv_data;

loff_t mychardev_llseek (struct file *filp, loff_t offset, int whence)
{
#if 0
	loff_t temp = 0;
	struct dev_private_data_mychr *mychardev_data = (struct dev_private_data_mychr*)filp->private_data;
	int maxsize = mychardev_data->size;
	pr_info("llseek requested\n");
	pr_info("Current file position : %lld\n", filp->f_pos);
	
	switch(whence)
	{
		case SEEK_SET:
			if( (offset > maxsize) || (offset < 0) ){
				return -EINVAL;
			}
			filp -> f_pos = offset;
			break;
			
		case SEEK_CUR:
			temp = filp -> f_pos + offset;
			if( (temp > maxsize) || (temp < 0) ){
				return -EINVAL;
			}
			filp -> f_pos = temp;
			break;
			
		case SEEK_END:
			temp = maxsize + offset;
			if( (offset > maxsize) || (offset < 0) ){
				return -EINVAL;
			}
			filp -> f_pos = temp;
			break;

		default:
			break;
	}
	pr_info("Updated file position : %lld\n", filp->f_pos);
	return filp->f_pos;
#endif
	return 0;
}

ssize_t mychardev_read (struct file *filp, char __user *buffer, size_t count, loff_t *f_pos)
{
#if 0
	int copystatus = 0;
	struct dev_private_data_mychr *mychardev_data = (struct dev_private_data_mychr*)filp->private_data;
	int maxsize = mychardev_data->size;
	pr_info("read requested for %zu bytes\n", count);
	pr_info("Current file position : %lld\n", *f_pos);
	
	/*1. Adjust the count*/
	if((*f_pos + count) > maxsize){
		count = maxsize - *f_pos;
	}
	
	/*2. Copy to user*/
	copystatus = copy_to_user(buffer, mychardev_data->buffer+(*f_pos), count);
	
	if(copystatus){
		pr_info("Some bytes could not be copied\n");
		return -EFAULT;
	}
	
	/*1. Update the current file position*/
	*f_pos = *f_pos + count;
	
	pr_info("Number of bytes successfully read : %zu\n", count);
	pr_info("Updated file position : %lld\n", *f_pos);
#endif
	return count;
}

ssize_t mychardev_write (struct file *filp, const char __user *buffer, size_t count, loff_t *f_pos)
{
#if 0
	int writestatus = 0;
	struct dev_private_data_mychr *mychardev_data = (struct dev_private_data_mychr*)filp->private_data;
	int maxsize = mychardev_data->size;
	pr_info("Write requested for %zu bytes\n", count);
	pr_info("File position before writing: %lld\n", *f_pos); 
	
	if((*f_pos + count) > maxsize){
		count = maxsize - *f_pos;
	}
	if(count == 0){
		pr_info("Buffer overflow\n");
		return -ENOMEM;
	}
	writestatus = copy_from_user(mychardev_data->buffer+(*f_pos), buffer, count);
	
	if(writestatus){
		pr_info("Some issues is writing to device memory\n");
		goto out;
	}
	
	*f_pos = *f_pos + count;
	
	pr_info("Number of bytes successfully written %zu\n", count);
	pr_info("Updated file position: %lld", *f_pos);
out:
#endif
	return count;
}

int check_permission(int dev_perm, int file_acc_mode);


int check_permission(int dev_perm, int file_acc_mode)
{
	if(dev_perm == RDWR){
		return 0;
	}
	/*Ensures read only access*/
	if( (dev_perm == RDONLY) && ( (file_acc_mode & FMODE_READ) && !(file_acc_mode & FMODE_WRITE) ) ){
		return 0;
	}
	/*Ensures write only access*/
	if( (dev_perm == WRONLY) && ( (file_acc_mode & FMODE_WRITE) && !(file_acc_mode & FMODE_READ) ) ){
		return 0;
	}
	return -EPERM;
}

int mychardev_open (struct inode *inode, struct file *filp)
{
	int ret = 0;
#if 0
	int minor_n;
	struct dev_private_data_mychr *mycharprdev_data;
	
	/*find out which device file open was attempted by userspace*/
	minor_n = MINOR(inode->i_rdev);
	pr_info("minor access = %d\n", minor_n);
	
	/*get device's private data structure here we have 0 to 3 totally 4 devices 
	so the below container_of macro return the exact address of specific device structure*/
	mycharprdev_data = container_of(inode->i_cdev, struct dev_private_data_mychr, cdev_chr);
	
	/*to supply device private data to other methods of the drivers; others methods dont have any chance to 
	parent structure of the member found using container of above, so store it in private_data field of file pointer*/
	filp->private_data = mycharprdev_data;
	
	/*check permission*/
	ret = check_permission(mycharprdev_data->perm, filp->f_mode);
	
	(!ret) ? pr_info("open was successful\n") : pr_info("open was unsuccessful\n");
#endif
	return ret;
}

int mychardev_release (struct inode *inode, struct file *filp)
{
	pr_info("Release was successful\n");
	return 0;
}

int mychar_platform_driver_probe(struct platform_device *mychardev)
{
	int ret;
	struct dev_private_data_mychr *dev_prv_data;
	struct dev_platform_data_mychr *pltfdata;
	
	/*for each device the probe will be called*/
	pr_info("Device is detected\n");
	
	/*1. Get the platform data */
	/*pltfdata = mychardev->dev.platform_data; -> also be obtained from below, member available in struct device*/
	pltfdata = (struct dev_platform_data_mychr*)dev_get_platdata(&mychardev->dev);
	if(!pltfdata){
		pr_err("Unable to retrieve the platform data of the device\n");
		ret = -EINVAL;
		goto out;
	}
	
	/*2. Dynamically allocate memory for the device private data */
	dev_prv_data = devm_kzalloc(&mychardev->dev, sizeof(*dev_prv_data), GFP_KERNEL);
	/*transition from kmalloc(2paramters) to devm_kalloc(3parameters) doing so 
	will prevent us from using kfree to manually free the memory unless there is an exception, if there is an exception using
	devm_kfree is a must*/
	if(!dev_prv_data){
		pr_info("Cannot allocate memory\n");
		ret = -ENOMEM;
		goto out;
	}
	
	/*save the device private data pointer in platform device structure */
	/*mychardev->dev.driver_data = dev_prv_data; can also use kernel api as below*/
	dev_set_drvdata(&mychardev->dev, dev_prv_data);
	
	dev_prv_data->mychrdata.size = pltfdata->size;
	dev_prv_data->mychrdata.perm = pltfdata->perm;
	dev_prv_data->mychrdata.serial_number = pltfdata->serial_number;
	
	pr_info("Device Serial Number = %s\n", dev_prv_data->mychrdata.serial_number);
	pr_info("Device Size = %d\n", dev_prv_data->mychrdata.size);
	pr_info("Device Permission = %d\n", dev_prv_data->mychrdata.perm);
	
	/*3. Dynamically allocate memory for the device buffer using size 
	information from the platform data */
	dev_prv_data->buffer = devm_kzalloc(&mychardev->dev, dev_prv_data->mychrdata.size, GFP_KERNEL);
	if(!dev_prv_data->buffer){
		pr_info("Cannot allocate memory\n");
		ret = -ENOMEM;
		goto dev_prv_data_free;
	}
	
	/*4. Get the device number */
	dev_prv_data->dev_num = drv_prv_data.device_num_base + mychardev->id;
	
	/*5. Do cdev init and cdev add */
	cdev_init(&dev_prv_data->cdev_chr, &dev_fops_mychr);
	dev_prv_data->cdev_chr.owner = THIS_MODULE; /*to prevent unwanted unloading of module where dev structure is declared*/
	
	ret = cdev_add(&dev_prv_data->cdev_chr, dev_prv_data->dev_num, 1);
	if(ret < 0){
		pr_err("Cdev add failed\n");
		goto buffer_free;
	}
	
	/*6. Create device file under /sys/class/mychardevice_class for the detected platform devices */
	drv_prv_data.device_mychardev = device_create(drv_prv_data.class_mychardev, NULL, dev_prv_data->dev_num, NULL, "mychardev-%d", mychardev->id);
	if(IS_ERR(drv_prv_data.device_mychardev)){
		ret = PTR_ERR(drv_prv_data.device_mychardev);
		pr_err("Device Creation Failed\n");
		goto cdev_del;
	}
	
	/*increment the total_devices once the probe function is triggered after device detected */
	drv_prv_data.total_devices++;
	pr_info("Probe was successfull");
	
	return 0;
	
	/*7. Error Handling */
cdev_del:
	cdev_del(&dev_prv_data->cdev_chr);
buffer_free:
	devm_kfree(&mychardev->dev, dev_prv_data->buffer);

dev_prv_data_free:
	devm_kfree(&mychardev->dev, dev_prv_data);
	
out:
	pr_info("Device Probe Failed\n");
	return ret;
}

/*Allowing the driver to unbind the device before releasing or cleaning up resources*/
int mychar_platform_driver_remove(struct platform_device *mychardev)
{
	struct dev_private_data_mychr *devprvdata;

	devprvdata = dev_get_drvdata(&mychardev->dev);/*using api to get the device private data stored in struct device contained in platform_device*/
	
	/*1. Remove a device that was created with device_create() */
	device_destroy(drv_prv_data.class_mychardev, devprvdata->dev_num);
	
	drv_prv_data.total_devices--; /*decrement the device count as the device gets removed */
	
	/*2. Remove a cdev entry */
	cdev_del(&devprvdata->cdev_chr);
	
	/*Free the memory allocated for buffer and the structure which contains it- struct 'device_prv_data_mychr'*/
	/*kfree(devprvdata->buffer);
	kfree(devprvdata); commented due to usage resource management api devm_kzalloc, kernel will handle the cleanup*/
	
	pr_info("Device Removed");
	return 0;
}

struct platform_driver platform_mychardrv = {
	.probe = mychar_platform_driver_probe,
	.remove = mychar_platform_driver_remove,
	.driver = {
		.name = "mychar_device"
	}
};

#define MAX_DEVICES 10
static int __init char_driver_start(void)
{
	int ret;
	
	/*1.Dynamically allocate a device number for MAX_DEVICES*/
	ret = alloc_chrdev_region(&drv_prv_data.device_num_base, 0, MAX_DEVICES, "mychardevices");
	if(ret < 0){
		pr_info("Device number allocation failed\n");
		return ret;
	}
	
	drv_prv_data.class_mychardev = class_create("mychardevice_class");
	if(IS_ERR(drv_prv_data.class_mychardev)){
		ret = PTR_ERR(drv_prv_data.class_mychardev);
		pr_err("Class Creation Failed\n");
		unregister_chrdev_region(drv_prv_data.device_num_base, MAX_DEVICES);
		return ret;
	}
	
	/*Register a platform device*/
	platform_driver_register(&platform_mychardrv);
	
	pr_info("mychar platform driver loaded\n");
	
	return 0;
}

static void __exit char_driver_exit(void)
{
	platform_driver_unregister(&platform_mychardrv);
	class_destroy(drv_prv_data.class_mychardev);
	unregister_chrdev_region(drv_prv_data.device_num_base, MAX_DEVICES);
	pr_info("mychar platform driver unloaded\n");
}

module_init(char_driver_start);
module_exit(char_driver_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("marzjukm7@gmail.com");
MODULE_DESCRIPTION("Simple char device implementation of mine");
MODULE_INFO(BOARD, "RASPBERRYPI4");
