#include <stdlib.h>
#include <linux/kernel.h>

#define container_of(ptr, type, member) ({ ( (void*)ptr - (void*)&((type*)0)->member  })
struct some_data
{
	char a;
	int b;
	char c;
	int d;
};
struct some_data data;

/*a function may be defined in some other file*/
void get_container(char *ptr)
{
	/*by using 'ptr', use the address of 'data' container*/
	struct some_data *pdata = container_of(ptr, struct some_data, c);
}
int init()
{
	data.a = 10;
	data.b = 5;
	data.c = 'a';
	data.d = 100;
	//passing member's address
	get_container(&data.c);
	r
