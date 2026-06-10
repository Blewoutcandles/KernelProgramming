#include <sys/types.h> 
#include <sys/stat.h> 
#include <fcntl.h>
#include<unistd.h>
#include<errno.h>
#include<stdio.h>
#include<stdlib.h>

char buffer[2048];

#define TRY_READ 10

int main(int argc, char *argv[])
{
	int fd;
	
	/*this variable holds remaining data bytes to be read */
	int remaining = TRY_READ;
	
	/*Holds count of total data bytes read so far */
	int total_read=0;
	
	int n =0,ret=0;

	if(argc != 2 ){
		printf("Wrong usage\n");
		printf("Correct usage: <file> <readcount>\n");
		return 0;
	}


	/*convert command line supplied data to integer */
	remaining = atoi(argv[1]);

	printf("read requested = %d\n",remaining);


	fd = open("/dev/mychardev-3",O_RDONLY);

	if(fd < 0){
		/*perror decodes user space errno variable and prints cause of failure*/
		perror("open");
		return fd;
	}

	printf("open was successful\n");

#if  1
	/*activate this for lseek testing */
	ret = lseek(fd,10,SEEK_SET);
	if(ret < 0){
		perror("lseek");
		close(fd);
		return ret;
	}
#endif
#if 0
	ret = lseek(fd,10,SEEK_END);
	if(ret < 0){
		perror("lseek");
		close(fd);
		return ret;
	}
#endif
	/*Lets attempt reading twice */
	
	while(n != 2 && remaining)
	{
		/*read data from 'fd' */
		ret = read(fd,&buffer[total_read],remaining);

		if(!ret){
			/*There is nothing to read */
			printf("end of file \n");
			break;
		}else if(ret <= remaining){
			printf("read %d bytes of data \n",ret );
			/*'ret' contains count of data bytes successfully read , so add it to 'total_read' */
		        total_read += ret;
			/*We read some data, so decrement 'remaining'*/
			remaining -= ret;
		}else if(ret < 0){
			printf("something went wrong\n");
			break;
		}else
			break;

		n++;
	}

	printf("total_read = %d\n",total_read);

	//dump buffer
	for(int i=0 ; i < total_read ; i++)
		printf("%c",buffer[i]);
	printf("\n");
	close(fd);	
	return 0;
}
/*t@takeoff-ASUS-TUF-Gaming-F15-FX506LH-FX566LH:/home/takeoff/code/Rspi_linux/drivers/char/002Pseudo_char_driver_multiple# gcc dev_read.c -o dev_read
root@takeoff-ASUS-TUF-Gaming-F15-FX506LH-FX566LH:/home/takeoff/code/Rspi_linux/drivers/char/002Pseudo_char_driver_multiple# echo "Can you show the current strace output and the kernel log (dmesg | tail -20) after attempting the write? That will let me verify whether the permission function is actually returning -EPERM or whether the access mode check itself is failing." > /dev/mychardev-3
root@takeoff-ASUS-TUF-Gaming-F15-FX506LH-FX566LH:/home/takeoff/code/Rspi_linux/drivers/char/002Pseudo_char_driver_multiple# dmesg | tail
[17937.846356] char_driver_start: Device number <major>:<minor> = 511:2
[17937.846492] char_driver_start: Device number <major>:<minor> = 511:3
[17937.846656] char_driver_start: Module init was successful
[18134.313024] mychardev_open: minor access = 2
[18134.313060] mychardev_open: open was successful
[18134.313122] mychardev_write: Write requested for 242 bytes
[18134.313127] mychardev_write: File position before writing: 0
[18134.313132] mychardev_write: Number of bytes successfully written 242
[18134.313136] mychardev_write: Updated file position: 242
[18134.313160] mychardev_release: Release was successful
root@takeoff-ASUS-TUF-Gaming-F15-FX506LH-FX566LH:/home/takeoff/code/Rspi_linux/drivers/char/002Pseudo_char_driver_multiple# ./dev_read 
Wrong usage
Correct usage: <file> <readcount>
root@takeoff-ASUS-TUF-Gaming-F15-FX506LH-FX566LH:/home/takeoff/code/Rspi_linux/drivers/char/002Pseudo_char_driver_multiple# ./dev_read 10000
read requested = 10000
open: Operation not permitted
root@takeoff-ASUS-TUF-Gaming-F15-FX506LH-FX566LH:/home/takeoff/code/Rspi_linux/drivers/char/002Pseudo_char_driver_multiple# gcc dev_read.c -o dev_read
root@takeoff-ASUS-TUF-Gaming-F15-FX506LH-FX566LH:/home/takeoff/code/Rspi_linux/drivers/char/002Pseudo_char_driver_multiple# ./dev_read 10000
read requested = 10000
open was successful
read 1024 bytes of data 
end of file 
total_read = 1024
Can you show the current strace output and the kernel log (dmesg | tail -20) after attempting the write? That will let me verify whether the permission function is actually returning -EPERM or whether the access mode check itself is failing.
root@takeoff-ASUS-TUF-Gaming-F15-FX506LH-FX566LH:/home/takeoff/code/Rspi_linux/drivers/char/002Pseudo_char_driver_multiple# */
