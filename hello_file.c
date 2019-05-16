#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <asm/uaccess.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Maxence Moutoussamy <maxence.moutoussamy1@gmail.com>");
MODULE_DESCRIPTION("This module is a pseudo device file that will contains an infinite number of \"Hello, file!\\n\"");
MODULE_VERSION("1.0");

#define MODULE_NAME "hello_file"
#define MODULE_REGION_NAME MODULE_NAME "_proc"
#define MODULE_CLASS_NAME MODULE_NAME "_sys"
#define MODULE_DEVICE_NAME MODULE_NAME "_dev"

// prototypes
static ssize_t	hello_file_read(struct file *, char __user *, size_t, loff_t *);
static int 	hello_file_open(struct inode *, struct file *);
static int	hello_file_release(struct inode *, struct file *);

// file operations definition
static const struct file_operations fops = {
	.owner 	 = THIS_MODULE,
	.read 	 = hello_file_read,
	.open 	 = hello_file_open,
	.release = hello_file_release,
};

// global state
static int 		handler_count = 0;
static dev_t 		dev;
static struct class*	class = NULL;
static struct cdev 	cdev;

static ssize_t hello_file_read(struct file *file, char __user *dest, size_t len, loff_t *offset)
{
	static const size_t 	mesg_len = 14;
	static const char 	mesg[14] = "Hello, file!\n";
	static ssize_t 		index = 0;

	int 			dest_index;
	
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
	printk(KERN_NOTICE "%s opened\n", MODULE_NAME);
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
	printk(KERN_NOTICE "%s released\n", MODULE_NAME);
	return (0);
}

static __init int hello_file_init(void)
{
	int 		err;
	struct device* 	device;

	// Allocates a place in /proc/devices
	err = alloc_chrdev_region(&dev, 0, 1, MODULE_REGION_NAME);
	if (err)
	{
		printk(KERN_ERR "couldn't alloc_chrdev_region: %d\n", err);
		return (err);
	}

	// Creates /sys/class entry
	class = class_create(THIS_MODULE, MODULE_CLASS_NAME);
	if (IS_ERR(class))
	{
		unregister_chrdev_region(dev, 1);
		printk(KERN_ERR "couldn't class_create: %ld\n", PTR_ERR(class));
		return (PTR_ERR(class));
	}

	cdev_init(&cdev, &fops);
	err = cdev_add(&cdev, dev, 1);
	if (err < 0)
	{
		class_destroy(class);
		unregister_chrdev_region(dev, 1);
		printk(KERN_ERR "couldn't cdev_add: %d\n", err);
		return (err);
	}

	// Creates /dev entry
	device = device_create(class, NULL, dev, NULL, MODULE_DEVICE_NAME);
	if (IS_ERR(device))
	{
		cdev_del(&cdev);
		class_destroy(class);
		unregister_chrdev_region(dev, 1);
		printk(KERN_ERR "couldn't device_create: %ld\n", PTR_ERR(device));
		return (PTR_ERR(device));
	}

	printk(KERN_INFO "%s is started\n", MODULE_NAME);
	return (0);
}

static __exit void hello_file_exit(void)
{
	device_destroy(class, dev);
	cdev_del(&cdev);
	class_destroy(class);
	unregister_chrdev(dev, MODULE_NAME);

	printk(KERN_INFO "%s is closed\n", MODULE_NAME);
}

module_init(hello_file_init);
module_exit(hello_file_exit);
