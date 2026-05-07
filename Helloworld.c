/*HEADER SECTION*/
#include <linux/modules.h> 

/*MODULE INITIALIZATION ENTRY POINT*/
static int __init my_k_module_init(void){
    pr_info("Hello World\n");
    return 0;
}

/*MODULE INITIALIZATION EXIT POINT RESOURCE CLEANING POINT*/
static void __exit my_k_module_exit(void){
    pr_info("Good bye world\n");
}

/*THIS IS THE REGISTRATION OF THE ABOVE ENTRY POINTS TO THE KERNEL*/
module_init(my_k_module_init);
module_exit(my_k_module_exit);

/*DESCRIPTION INFORMATION ABOUT THE MODULE*/
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Marzjuk");
MODULE_DESCRIPTION("THIS IS MY FIRST KERNEL MODULE");

/*Note:
* Marked static and __init - so neither other modules call this module entry point nor it provides any support or 
* services to other modules, for static modules: 
* function exit point will not get called since we cannot remove a statically linked module even the kernel ignores 
* the function that has __exit marker when during build process hence it makes sense to make this function static 
* though it is optional but dynamic modules can be inserted or removed during runtime as per user commands
* __exit: This is an entry point to modules removal. Clean up function will get called only in case of dynamic modules
* and removed using rmmod commands
* Kernel header files location: 
* linux_source/include/linux
* FN MACROS:
* __init, __exit
* How these are defined in the files:
* #define __init        __section(.init.text)  -->compiler directive which directs the compiler to keep data or code in
*                                                   an output section called ".init"
* #define __initdata    __section(.init.data)
* #define __initconst   __section(.init.rodata)
* #define __exit        __section(.exit.text) --> compiler directive which directs the compiler to keep data or code in
*                                                   an output section called ".exit"
* defined in linux_src/include/linux/init.h
*/

