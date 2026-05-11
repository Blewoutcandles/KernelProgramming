/*HEADER SECTION*/
#include <linux/init.h>
#include <linux/module.h>

/*MODULE INITIALIZATION ENTRY POINT*/
static int __init my_k_module_begin(void)
{
    pr_info("Hello World\n");
    return 0;
}

/*MODULE INITIALIZATION EXIT POINT RESOURCE CLEANING POINT*/
static void __exit my_k_module_exit(void)
{
    pr_info("Good bye world\n");
}

/*THIS IS THE REGISTRATION OF THE ABOVE ENTRY POINTS TO THE KERNEL*/
module_init(my_k_module_begin);
module_exit(my_k_module_exit);

/*DESCRIPTION INFORMATION ABOUT THE MODULE*/
MODULE_LICENSE("GPL");
MODULE_AUTHOR("marzjukm7@gmail.com");
MODULE_DESCRIPTION("THIS IS MY FIRST KERNEL MODULE");

