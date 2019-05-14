#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <asm/uaccess.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Maxence Moutoussamy <maxence.moutoussamy1@gmail.com>");
MODULE_DESCRIPTION("This module is a pseudo device file that will contains an infinite number of \"Hello, file!\\n\"");
MODULE_VERSION("1.0");

// prototypes
static ssize_t	hello_file_read(struct file *, char __user *, size_t, loff_t *);
static int 	hello_file_open(struct inode *, struct file *);
static int	hello_file_release(struct inode *, struct file *);

// file operations definition
static const struct file_operations fops = {
	.read = hello_file_read,
	.open = hello_file_open,
	.release = hello_file_release,
};

// global state
static int handler_count = 0;
static int major;
static const char* module_name = "hello-file";

static ssize_t hello_file_read(struct file *file, char __user *dest, size_t len, loff_t *offset)
{
	static const ssize_t mesg_len = 14;
	static const char mesg[14] = "Hello, file!\n";
	static ssize_t index = 0;

	int dest_index;
	
	printk(KERN_DEBUG "message: %s; len: %lu; idx: %lu\n", mesg, mesg_len, index);

	for (dest_index = 0; dest_index < len; dest_index += 1, index = (index + 1) % mesg_len)
	{
		if (put_user(mesg[index], &(dest[index])) == -EFAULT)
		{
			printk(KERN_WARNING "EFAULT error happened during copy to user space\n");
			return (-EFAULT);
		}
	}

	return (len);
}

static int hello_file_open(struct inode *inode, struct file *file)
{
	if (handler_count)
	{
		return (-EBUSY);
	}

	handler_count += 1;
	try_module_get(THIS_MODULE);
	printk(KERN_NOTICE "%s opened\n", module_name);
	return (0);
}

static int hello_file_release(struct inode *inode, struct file *file)
{
	if (!handler_count)
	{
		return (-EBADF);
	}

	handler_count -= 1;
	module_put(THIS_MODULE);
	printk(KERN_NOTICE "%s released\n", module_name);
	return (0);
}

static __init int hello_file_init(void)
{
	major = register_chrdev(0, module_name, &fops);
	if (major == -EINVAL)
	{
		printk(KERN_ERR "The given major number is not valid\n");
		return (major);
	}
	else if (major == -EBUSY)
	{
		printk(KERN_ERR "The given major number is busy\n");
		return (major);
	}

	printk(KERN_INFO "%s is started\n", module_name);
	return (0);
}

static __exit void hello_file_exit(void)
{
	unregister_chrdev(major, module_name);
	printk(KERN_INFO "%s is closed\n", module_name);
}

module_init(hello_file_init);
module_exit(hello_file_exit);
