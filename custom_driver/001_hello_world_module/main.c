#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>

#define AUTHOR "Phong"
//
static int __init hello_init(void)
{
	printk(KERN_INFO"Hello World!\n");
	return 0;
}

static void __exit hello_cleanup(void)
{
	printk(KERN_INFO"Exit hello world module!\n");
}

module_init(hello_init);
module_exit(hello_cleanup);

MODULE_LICENSE("GPL");
MODULE_AUTHOR(AUTHOR);
MODULE_DESCRIPTION("Hello world module");
