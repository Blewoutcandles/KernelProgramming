#include <stdlib.h>
#include <stdio.h>

#define container_of(ptr, type, member) ({ (type*) ((void*)ptr - (void*)&((type*)0)->member); })
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
	printf("a = %d, b = %d, c = %c, d = %d\n", pdata->a, pdata->b, pdata->c, pdata->d);
	return;
}
int main()
{
	data.a = 10;
	data.b = 5;
	data.c = 'a';
	data.d = 100;
	//passing member's address
	get_container(&data.c);
	return 0;
}
