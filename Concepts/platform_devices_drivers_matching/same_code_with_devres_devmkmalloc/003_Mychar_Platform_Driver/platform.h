/*These are device specific information refer 002Pseudo_mychar_driver_multiple.c private data structure*/

#define RDWR	0x11
#define RDONLY	0X01
#define WRONLY	0X10

struct dev_platform_data_mychr
{
	int size;
	int perm;
	const char *serial_number;
};
