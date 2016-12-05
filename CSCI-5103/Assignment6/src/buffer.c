#include <linux/wait.h>		// for wait_queue_head_t 
#include <linux/moduleparam.h>	// for module_param
#include <linux/stat.h>		// for permission values
#include <linux/slab.h>		// for kmalloc, kfree, ksize
#include <linux/fs.h>		// for fmode, FMODE_READ, FMODE_WRITE, nonseekable_open
#include <linux/export.h>	// for THIS_MODULE?	
#include <linux/cdev.h>		// for cdev
#include <linux/module.h>	// for this_module
#include <linux/gfp.h>		// for GFP* flags
#include <linux/init.h>		// for module_init, module_exit
#include <linux/semaphore.h>	// for init_MUTEX
//#include "scull.h"		// local definitions

#define init_MUTEX(_m) sema_init(_m, 1);

struct scull_buffer
{
	wait_queue_head_t producer_queue, consumer_queue; /* producer and consumer queues */
        char *buffer, *buffer_end;              /* begin of buffer, end of buffer */
        int buffer_size;                    	/* for pointer arithmetic */
        char *rp, *wp;                     	/* read address, write address*/
        int num_producers, num_consumers;       /* number of openings for p/c */
        struct semaphore sem;              	/* mutual exclusion semaphore */
        struct cdev cdev;                  	/* Char device structure */
};

// parameters
static int device_count = 1;	// number of scull_buffer devices
static int nitems = 20;		// default value for size of buffer
static int item_size = 32;	// 32B
int major = 0;
int minor;

module_param(device_count, int, S_IRUGO);
module_param(nitems, int, S_IRUGO);
module_param(item_size, int, S_IRUGO);
module_param(major, int, S_IRUGO);
module_param(minor, int, S_IRUGO);

// create device number structure
// create pointer to devices
dev_t scullb_devno;				// first device # in list
static int spacefree(struct scull_buffer *dev);	// TODO: ?

MODULE_AUTHOR("Maxwell Pung");
MODULE_LICENSE("GPL");

// open	
// where inode is a pointer to...
// where filp is a pointer to a struct file
static int scullb_open(struct inode *inode, struct file *filp)
{
	struct scull_buffer *dev;

	dev = container_of(inode->i_cdev, struct scull_buffer, cdev);	// find appropriate device structure
	filp->private_data = dev;	// for easy access to scull_buffer struct in future

	if (down_interruptible(&dev->sem))
	{
		return -ERESTARTSYS;
	}
	// TODO the block below should be moved to the init function
	if (dev->buffer == NULL)	// then the buffer hasn't been created, so create it.
	{
		dev->buffer = kmalloc(nitems, GFP_KERNEL);	// TODO: verify size is correct
                if (dev->buffer == NULL)	// then memory was unsuccessfully allocated.
		{
                        up(&dev->sem);
                        return -ENOMEM;
                }
		printk(KERN_ALERT "Buffer has been allocated %zu bytes.\n", ksize(dev->buffer));
	}

	// assign sh to other scull_buffer members

 	// determine if calling process is consumer or producer & add it to respective counter
	if (filp->f_mode & FMODE_READ)
	{
                dev->num_consumers++;
		printk(KERN_ALERT "Scullbuffer: There are now %d consumers.\n", dev->num_consumers++);
	}
        if (filp->f_mode & FMODE_WRITE)
	{
                dev->num_producers++;
		printk(KERN_ALERT "Scullbuffer: There are now %d producers.\n", dev->num_producers++);

	}
        up(&dev->sem);
        return nonseekable_open(inode, filp);	// seeking is not necessary
}

// release
// I believet this function should combine scull_p_release & scull_p_cleanup.
static int scullb_release(struct inode *inode, struct file *filp)
{
	struct scull_buffer *dev = filp->private_data;

        down(&dev->sem);
        if (filp->f_mode & FMODE_READ)		// then a consumer has finished consuming
	{	
                dev->num_consumers--;
		printk(KERN_ALERT "Scullbuffer: There are now %d consumers.\n", dev->num_consumers++);

	}
        if (filp->f_mode & FMODE_WRITE)		// then a producer has finished producing
	{
                dev->num_producers--;
		printk(KERN_ALERT "Scullbuffer: There are now %dproducers.\n", dev->num_producers++);

	}
        if (dev->num_consumers + dev->num_producers == 0) 	// then all producers & consumers are finished w/ their work
	{
                kfree(dev->buffer);
                dev->buffer = NULL;
        }
        up(&dev->sem);
        return 0;
}

// read 
static ssize_t scullb_read(struct file *filp, char __user *buf, size_t count, loff_t *f_pos)
{
	/*struct scull_buffer *dev = filp->private_data;

	// Acquire semaphore unless interrupted.
	if (down_interruptible(&dev->sem))	// Then semaphore was not acquired successfully.
	{
		return -ERESTARTSYS;		// This system call can be restarted.
	}

	// if no data in input buffer & producer procs have scullbuffer open, block until atleast one byte is there
	// if no data in input buffer & no producer procs have scullbuffer open, return 0
	while ()
	{

	}
	// if EOF, return immeditaely w/ 0
	// if data in input buffer, return immediately*/
}

// write
// never make write call wait for data transmission before returning
static ssize_t scullb_write(struct file *filp, const char __user *buf, size_t count, loff_t *f_pos)
{
	// if space in output buffer, return w/ out delay (must accept atleast 1 byte)
	// if output buffer full, block until some space is freed
}

struct file_operations fops = 
{
	.owner = THIS_MODULE,
	.read = scullb_read, 
	.write = scullb_write, 
	.open = scullb_open, 
	.release = scullb_release, 
};

// Set up a cdev entry. TODO: index necessary?
static void scullb_setup_cdev(struct scull_buffer *dev/*, int index*/)
{
	int err, devno = scullb_devno/* + index*/;
    
	cdev_init(&dev->cdev, &fops);
	dev->cdev.owner = THIS_MODULE;
	err = cdev_add(&dev->cdev, devno, 1);

	// Fail gracefully if need be.
	if (err)
	{
		printk(KERN_NOTICE "Error %d adding scullbuffer.", err);
	}
}

static struct scull_buffer *scullb_device;	// pointer to device

int scullb_init(void)
{
	int result = -1;
	dev_t dev = 0;
/*
 * Get a range of minor numbers to work with, asking for a dynamic
 * major unless directed otherwise at load time.
 */
	if (major) 
	{
		dev = MKDEV(major, minor);
		result = register_chrdev_region(dev, device_count, "scullbuffer");
	} 
	else 
	{
		result = alloc_chrdev_region(&dev, minor, device_count, "scullbuffer");
		major = MAJOR(dev);
		}
	if (result < 0) 
	{
		printk(KERN_WARNING "scullbuffer: can't get major %d\n", major);
		// TODO release call necessarry?
		return result;
	}
        /* 
	 * allocate the devices -- we can't have them static, as the number
	 * can be specified at load time
	 */
	scullb_devno = dev;
	scullb_device = kmalloc(device_count * sizeof(struct scull_buffer), GFP_KERNEL);
	if (scullb_device == NULL) 
	{
		unregister_chrdev_region(dev, device_count);
		// TODO release call necessary?
		return -ENOMEM;
	}
	memset(scullb_device, 0, device_count * sizeof(struct scull_buffer));

        // Initialize each device.
	init_waitqueue_head(&scullb_device->producer_queue);
	init_waitqueue_head(&scullb_device->consumer_queue);
	scullb_device->num_producers = scullb_device->num_consumers = 0;
	scullb_device->buffer_size = nitems;
	scullb_device->buffer_end = scullb_device->buffer + scullb_device->buffer_size;
	scullb_device->rp = scullb_device->wp = scullb_device->buffer;
	init_MUTEX(&scullb_device->sem);
	scullb_setup_cdev(scullb_device);

	dev = MKDEV(major, minor + device_count);	// TODO necessary?
	return 0; /* succeed */
}

module_init(scullb_init);
module_exit(scullb_release);	
