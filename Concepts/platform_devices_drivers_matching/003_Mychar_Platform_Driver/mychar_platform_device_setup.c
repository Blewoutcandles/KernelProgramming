#include <linux/module.h>
#include <linux/platform_device.h>
#include "platform.h"

#undef pr_fmt
#define pr_fmt(fmt) 		"%s: "fmt, __func__

void mychardev_release(struct device* dev);

/*Create 2 platform data for 2 devices*/
struct dev_platform_data_mychr mychardev_pdata[4] = {
	[0] = { .size = 512, .perm = RDWR, .serial_number = "MYCHARPLATFORMDEV1"},
	[1] = { .size = 512, .perm = RDWR, .serial_number = "MYCHARPLATFORMDEV2"},
	[2] = { .size = 512, .perm = RDONLY, .serial_number = "MYCHARPLATFORMDEV3"},
	[3] = { .size = 512, .perm = WRONLY, .serial_number = "MYCHARPLATFORMDEV4"}
};

void mychardev_release(struct device* dev){
	pr_info("Device released\n");
}

/*1. Create 2 platform devices.*/
struct platform_device platform_mychardev_1 = {
	.name = "mychar_device",
	.id = 0,
	.dev = {	
			.platform_data = &mychardev_pdata[0],
			.release = mychardev_release,
   	}
};

struct platform_device platform_mychardev_2 = {
	.name = "mychar_device",
	.id = 1,
	.dev = {	
			.platform_data = &mychardev_pdata[1],
			.release = mychardev_release,
   	}
};

struct platform_device platform_mychardev_3 = {
	.name = "mychar_device",
	.id = 2,
	.dev = {	
			.platform_data = &mychardev_pdata[2],
			.release = mychardev_release,
   	}
};

struct platform_device platform_mychardev_4 = {
	.name = "mychar_device",
	.id = 3,
	.dev = {	
			.platform_data = &mychardev_pdata[3],
			.release = mychardev_release,
   	}
};

struct platform_device *pltf_mydevices[] = {
	&platform_mychardev_1,
	&platform_mychardev_2,
	&platform_mychardev_3,
	&platform_mychardev_4
};

static int __init mychar_platform_init(void)
{
	/*platform_device_register(&platform_mychardev_1);
	platform_device_register(&platform_mychardev_2); all devices included in the below platform_add_devices() API*/
	platform_add_devices(pltf_mydevices, ARRAY_SIZE(pltf_mydevices) );
	/*Check under /sys/devices/platform */
	pr_info("Device Setup module inserted\n");
	return 0;
}

static void __exit mychar_platform_exit(void)
{
	platform_device_unregister(&platform_mychardev_1);
	platform_device_unregister(&platform_mychardev_2);
	platform_device_unregister(&platform_mychardev_3);
	platform_device_unregister(&platform_mychardev_4);
	pr_info("Device Setup module removed\n");
}

module_init(mychar_platform_init);
module_exit(mychar_platform_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("marzjukm7@gmail.com");
MODULE_DESCRIPTION("Module which registers platform devices");
MODULE_INFO(BOARD, "Raspberrypi4");
