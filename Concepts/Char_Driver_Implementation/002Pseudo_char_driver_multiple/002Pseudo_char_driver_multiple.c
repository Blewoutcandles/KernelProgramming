/*GPL LICENSE 2.0*/
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/kdev_t.h>
#include <linux/uaccess.h>

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

/* file operations of the driver */
struct file_operations chrdev_fops = {
	.open = mychardev_open,
	.read = mychardev_read,
	.write = mychardev_write,
	.release = mychardev_release,
	.llseek = mychardev_llseek,
	.owner = THIS_MODULE
};
/*Device private data structure*/
struct mychardev_private_data{
    char* buffer;
    unsigned size;
    const char* serial_number;
    int perm;
    struct cdev chrdev_cdev;
};

/*Driver private data structure*/
struct mychardrv_private_data{
	int total_devices;
  	dev_t device_number;
 	struct class* class_mychardev;
	struct device* device_mychardev;
  	struct mychardev_private_data mychardev_data[NO_OF_DEVICES];
};

#define RDONLY	0x01
#define WRONLY	0x10
#define RDWR	0x11

/*Driver private data structure member initialization*/
struct mychardrv_private_data mychardrv_data = 
{
	.total_devices = NO_OF_DEVICES,
	.mychardev_data = {
		[0] = {
				.buffer = device_buffer_pcdev1,
				.size = MEM_SIZE_MAX_PCDEV1,
				.serial_number = "PCDEV1",
				.perm = RDONLY /*RDONLY*/,
			},
		[1] = {
				.buffer = device_buffer_pcdev2,
				.size = MEM_SIZE_MAX_PCDEV2,
				.serial_number = "PCDEV2",
				.perm = WRONLY /*WRONLY*/,
			},
		[2] = {
				.buffer = device_buffer_pcdev3,
				.size = MEM_SIZE_MAX_PCDEV3,
				.serial_number = "PCDEV3",
				.perm = RDWR /*RDWR*/,
			},
		[3] = {
				.buffer = device_buffer_pcdev4,
				.size = MEM_SIZE_MAX_PCDEV4,
				.serial_number = "PCDEV4",
				.perm = RDWR /*RDWR*/,
			}
	}
};

loff_t mychardev_llseek (struct file *filp, loff_t offset, int whence)
{
	loff_t temp = 0;
	struct mychardev_private_data *mychardev_data = (struct mychardev_private_data*)filp->private_data;
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
}

ssize_t mychardev_read (struct file *filp, char __user *buffer, size_t count, loff_t *f_pos)
{
	int copystatus = 0;
	struct mychardev_private_data *mychardev_data = (struct mychardev_private_data*)filp->private_data;
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
	return count;
}

ssize_t mychardev_write (struct file *filp, const char __user *buffer, size_t count, loff_t *f_pos)
{
	int writestatus = 0;
	struct mychardev_private_data *mychardev_data = (struct mychardev_private_data*)filp->private_data;
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
	int ret;
	int minor_n;
	struct mychardev_private_data *mycharprdev_data;
	
	/*find out which device file open was attempted by userspace*/
	minor_n = MINOR(inode->i_rdev);
	pr_info("minor access = %d\n", minor_n);
	
	/*get device's private data structure here we have 0 to 3 totally 4 devices 
	so the below container_of macro return the exact address of specific device structure*/
	mycharprdev_data = container_of(inode->i_cdev, struct mychardev_private_data, chrdev_cdev);
	
	/*to supply device private data to other methods of the drivers; others methods dont have any chance to 
	parent structure of the member found using container of above, so store it in private_data field of file pointer*/
	filp->private_data = mycharprdev_data;
	
	/*check permission*/
	ret = check_permission(mycharprdev_data->perm, filp->f_mode);
	
	(!ret) ? pr_info("open was successful\n") : pr_info("open was unsuccessful\n");
	
	return ret;
}

int mychardev_release (struct inode *inode, struct file *filp)
{
	pr_info("Release was successful\n");
	return 0;
}

static int __init char_driver_start(void)
{
	int ret, i;
	
	/*1. Dynamically allocate a device number */
	ret = alloc_chrdev_region(&mychardrv_data.device_number, 0, NO_OF_DEVICES, "mychardevice_devno");
	if(ret < 0){
		pr_info("Device number allocation failed\n");
		goto out;
	}
	
	/*2 Create a device class under /sys/class */
	//class_mychardev = class_create(THIS_MODULE, "mychardevice_class"); not compatible right now
	mychardrv_data.class_mychardev = class_create("mychardevice_class");
	if(IS_ERR(mychardrv_data.class_mychardev )){
		ret = PTR_ERR(mychardrv_data.class_mychardev );
		pr_err("Class creation failed\n");
		goto unreg_chardev;
	}
	
	for(i = 0; i < NO_OF_DEVICES; i++)
	{
		pr_info("Device number <major>:<minor> = %d:%d\n", MAJOR(mychardrv_data.device_number+i), MINOR(mychardrv_data.device_number+i));
		
		/*3. Initialize the cdev structure with fops and owner is set to current module*/
		cdev_init( 	&mychardrv_data.mychardev_data[i].chrdev_cdev, &chrdev_fops);
		mychardrv_data.mychardev_data[i].chrdev_cdev.owner = THIS_MODULE;
		
		/*4. Register a device (cdev structure) with VFS using below cdev_add*/
		ret = cdev_add(&mychardrv_data.mychardev_data[i].chrdev_cdev, mychardrv_data.device_number+i, 1);
		if(ret < 0){
			goto cdev_del;
		}
		
		/*5. Populate the sysfs with device information. Create a dev file under /sys/class/class_name(mychardevice_class)/ */
		mychardrv_data.device_mychardev = device_create(mychardrv_data.class_mychardev, NULL, mychardrv_data.device_number+i, NULL, "mychardev-%d", i+1); 
		if(IS_ERR(mychardrv_data.device_mychardev )){
			ret = PTR_ERR(mychardrv_data.device_mychardev );
			pr_err("Device Creation failed\n");
			goto dev_del;
		}
		
		/*mychardev will appear in dev file under /sys/class/mychardevice_class/*/
	}
	
	pr_info("Module init was successful\n");
	
	return 0;

dev_del:
cdev_del:
	pr_err("device destruction\n");
	pr_err("cdev destruction\n");
	for(; i >= 0; i--)
	{
		device_destroy(mychardrv_data.class_mychardev, mychardrv_data.device_number+i);
		cdev_del(&mychardrv_data.mychardev_data[i].chrdev_cdev);
		
	}
	pr_err("Class destruction\n");
	class_destroy(mychardrv_data.class_mychardev);

unreg_chardev:
	pr_err("unregister cdev\n");
	unregister_chrdev_region(mychardrv_data.device_number, NO_OF_DEVICES);
out:
	pr_info("Module initialization failed\n");
	return ret;
}


static void __exit char_driver_exit(void)
{
	int i;
	/* Resource cleanup should be done exactly in reverse order of init function */ 
	for(i = 0; i < NO_OF_DEVICES; i++)
	{
		device_destroy(mychardrv_data.class_mychardev, mychardrv_data.device_number+i);
		cdev_del(&mychardrv_data.mychardev_data[i].chrdev_cdev);
	}
	class_destroy(mychardrv_data.class_mychardev);
	unregister_chrdev_region(mychardrv_data.device_number, NO_OF_DEVICES);
	pr_info("Module exit was successful\n");
}

module_init(char_driver_start);
module_exit(char_driver_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("marzjukm7@gmail.com");
MODULE_DESCRIPTION("Simple char device implementation of mine");
MODULE_INFO(BOARD, "RASPBERRYPI4");
