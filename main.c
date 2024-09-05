#include <linux/init.h>
#include <linux/module.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/kernel.h>
#include <linux/uaccess.h>
#include <linux/fs.h>
#include <linux/version.h>

static int kwitter_open(struct inode *inode, struct file *file);
static int kwitter_release(struct inode *inode, struct file *file);
static ssize_t kwitter_read(struct file *file, char __user *buf, size_t count, loff_t *offset);
static ssize_t kwitter_write(struct file *file, const char __user *buf, size_t count, loff_t *offset);

static const struct file_operations kwitter_fops = {
	.open		= kwitter_open,
	.release	= kwitter_release,
	.read		= kwitter_read,
	.write		= kwitter_write
};

static int dev_major = 0;
static struct class *kwitter_class = NULL;
static struct cdev cdev;

static int __init kwitter_init(void)
{
	dev_t dev;

	int err = alloc_chrdev_region(&dev, 0, 1, "kwitter");
	if (err != 0) {
	return err;
	}

	dev_major = MAJOR(dev);

#if LINUX_VERSION_CODE > KERNEL_VERSION(6, 3, 0)
	kwitter_class = class_create("kwitter");
#else
	kwitter_class = class_create(THIS_MODULE, "kwitter");
#endif

	cdev_init(&cdev, &kwitter_fops);
	cdev.owner = THIS_MODULE;
	err = cdev_add(&cdev, MKDEV(dev_major, 0), 1);
	if (err != 0) {
	return err;
	}

	// create_device will add device to /dev and /sys directoreis,
	// otherwise you have to "mknod /dev/kwitter c dev_major 0" by hand.
	device_create(kwitter_class, NULL, MKDEV(dev_major, 0), NULL, "kwitter");

	printk("KWITTER: module loaded major: %u\n", dev_major);
	return 0;
}

static void __exit kwitter_exit(void)
{
	device_destroy(kwitter_class, MKDEV(dev_major, 0));
	class_unregister(kwitter_class);
	class_destroy(kwitter_class);
	unregister_chrdev_region(MKDEV(dev_major, 0), MINORMASK);

	printk("KWITTER: module unloaded\n");
}

static int kwitter_open(struct inode *inode, struct file *file)
{
	printk("KWITTER: Device open\n");
	return 0;
}

static int kwitter_release(struct inode *inode, struct file *file)
{
	printk("KWITTER: Device close\n");
	return 0;
}

#define MESSAGE "Tbl IIuDop!\n"
#define MESSAGE_SIZE strlen(MESSAGE)

static ssize_t kwitter_read(struct file *file, char __user *buf, size_t count, loff_t *offset)
{
	return simple_read_from_buffer(buf, count, offset, MESSAGE, MESSAGE_SIZE);
}

static ssize_t kwitter_write(struct file *file, const char __user *buf, size_t count, loff_t *offset)
{
	return count;
}

MODULE_LICENSE("GPL");
MODULE_AUTHOR("ALex Nikonov <alex@nikonov.tech>");
MODULE_DESCRIPTION("Kernel interface for russian-speaking part of Twitter social network.");

module_init(kwitter_init);
module_exit(kwitter_exit);
