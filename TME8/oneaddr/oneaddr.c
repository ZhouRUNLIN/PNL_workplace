#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/syscalls.h>


MODULE_DESCRIPTION("Module contenant ...");
MODULE_AUTHOR("Julien Sopena, LIP6");
MODULE_LICENSE("GPL");

int x[] = {50, 10, 82, 29, 24, 13, 16, 82, 3, 12, 2, 8, 27, 78, 6, 4, 24, 89,
	29, 8, 2, 31, 8, 18, 78, 6, 86, 8, 18, 26, 71, 18, 24, 31, 81, 23, 25,
	5, 11, 20, 77, 91, 67, 76, 81, 15, 14, 0, 15, 11, 25, 74, 10, 4, 24,
	17, 22, 24, 2, 15, 29, 65, 68, 80, 76, 0};
int y[] = {97, 99, 114, 107, 119, 120, 99, 114, 111, 101, 113, 109, 97, 110,
	101, 97, 107, 121, 113, 97, 101, 113, 109, 97, 110, 101, 113, 109, 97,
	110, 103, 112, 119, 113, 113, 100, 112, 98, 101, 113, 109, 97, 110,
	101, 113, 109, 97, 110, 97, 110, 106, 106, 120, 97, 110, 120, 101, 113,
	109, 97, 110, 97, 101, 113, 109, 0};
char z[1024];

static int __init my_secret_init(void)
{
	int i;

	for (i = 0 ; i < sizeof(x)/sizeof(int) ; i++)
		z[i] = x[i] ^ y[i];
	z[i] = 0;

	pr_info("Voilà une adresse : 0x%px\n", &z[0]);
	return 0;
}
module_init(my_secret_init);

static void __exit my_secret_exit(void)
{
	pr_info("Plus rien a cacher !!!!\n");
}
module_exit(my_secret_exit);