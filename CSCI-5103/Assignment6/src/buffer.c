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
#include <linux/sched.h>	// for signal_pending
//#include "scull.h"		// local definitions

#define init_MUTEX(_m) sema_init(_m, 1)
#define DEVICE_NAME "buffer"
int MAJOR_NUM = 247;
int MINOR_NUM = 0;
#define DEVICE_COUNT 1
#define NITEMS 20
#define BUF_SIZE 1024

MODULE_AUTHOR("Maxwell Pung");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("A character device driver that handles the bounded buffer problem.");
MODULE_SUPPORTED_DEVICE(DEVICE_NAME);

static struct scull_buffer *scullb_device;	// pointer to device
dev_t scullb_devno;				// first device # in list

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

static int spacefree(struct scull_buffer *dev);

/*
	Description: This function is called when a process attempts to open the device file.
*/
static int scullb_open(struct inode *inode, struct file *filp)
{
	struct scull_buffer *dev;
	dev = container_of(inode->i_cdev, struct scull_buffer, cdev);	// find appropriate device structure
	filp->private_data = dev;	// for easy access to scull_buffer struct in future

	printk(KERN_INFO "scullbuffer: entering critical region in open...\n");
	if (down_interruptible(&dev->sem))	
	{
		return -ERESTARTSYS;
	}

 	// determine if calling process is consumer or producer & add it to respective counter
	if (filp->f_mode & FMODE_READ)
	{
                dev->num_consumers++;
		printk(KERN_INFO "scullbuffer: There are now %d consumers.\n", dev->num_consumers);
	}
        if (filp->f_mode & FMODE_WRITE)
	{
                dev->num_producers++;
		printk(KERN_INFO "scullbuffer: There are now %d producers.\n", dev->num_producers);

	}
        up(&dev->sem);
	printk(KERN_INFO "scullbuffer: leaving critical region in open...\n");
        return nonseekable_open(inode, filp);	// seeking is not necessary
}

/*
	Definition: Called when process closes a device file.
*/
static int scullb_release(struct inode *inode, struct file *filp)
{
	struct scull_buffer *dev = filp->private_data;
        down(&dev->sem);
        if (filp->f_mode & FMODE_READ)		// then a consumer has finished consuming
	{	
                dev->num_consumers--;
		printk(KERN_INFO "scullbuffer: There are now %d consumers after release.\n", dev->num_consumers);

	}
        if (filp->f_mode & FMODE_WRITE)		// then a producer has finished producing
	{
                dev->num_producers--;
		printk(KERN_INFO "scullbuffer: There are now %d producers after release.\n", dev->num_producers);

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
	struct scull_buffer *dev = filp->private_data;

	// Acquire semaphore unless interrupted.
	if (down_interruptible(&dev->sem))	// Then semaphore was not acquired successfully.
	{
		return -ERESTARTSYS;		// This system call can be restarted.
	}

	// if no data in input buffer & producer procs have scullbuffer open, block until atleast one byte is there
	// if no data in input buffer & no producer procs have scullbuffer open, return 0
	while (dev->rp == dev->wp)
	{
		if (filp->f_flags & O_NONBLOCK)
		{
			return -EAGAIN;
		}
		printk(KERN_INFO "scullbuffer: consumer process is going to sleep...\n");
		if (wait_event_interruptible(dev->consumer_queue, (dev->rp != dev->wp)))
		{
			return -ERESTARTSYS;
		}
		if (down_interruptible(&dev->sem))
		{
			return -ERESTARTSYS;
		}
	}
	// if EOF, return immeditaely w/ 0
	// if data in input buffer, return immediately
	if (dev->wp > dev->rp)
	{
		count = min(count, (size_t)(dev->wp - dev->rp));
	}
	else
	{
		count = min(count, (size_t)(dev->buffer_end - dev->rp));
	}
	if (copy_to_user(buf, dev->rp, count))
	{
		up(&dev->sem);
		return -EFAULT;
	}
	dev->rp += count;
	if (dev->rp == dev->buffer_end)
	{
		dev->rp = dev->buffer;
	}
	up (&dev->sem);
	wake_up_interruptible(&dev->producer_queue);
	printk("scullbuffer: leaving read, read %li bytes...\n", (long)count);
	return count;
}

static int scullb_getwritespace(struct scull_buffer *dev, struct file *filp)
{
        while (spacefree(dev) == 0) 
	{ 
                DEFINE_WAIT(wait);

                up(&dev->sem);
                if (filp->f_flags & O_NONBLOCK)
		{
                        return -EAGAIN;
		}
                printk("scullbuffer: producer going to sleep in scull_getwritespace...\n");
                prepare_to_wait(&dev->producer_queue, &wait, TASK_INTERRUPTIBLE);
                if (spacefree(dev) == 0)
		{
                        schedule();
		}
                finish_wait(&dev->producer_queue, &wait);
                if (signal_pending(current))
		{
                        return -ERESTARTSYS; 
		}
                if (down_interruptible(&dev->sem))
		{
                        return -ERESTARTSYS;
		}
        }
	printk("scullbuffer: leaving scull_getwritespace after waiting...\n");
        return 0;
}


static int spacefree(struct scull_buffer *dev)
{
        if (dev->rp == dev->wp)
	{
                return dev->buffer_size - 1;
	}
        return ((dev->rp + dev->buffer_size - dev->wp) % dev->buffer_size) - 1;
}

// write
// never make write call wait for data transmission before returning
static ssize_t scullb_write(struct file *filp, const char __user *buf, size_t count, loff_t *f_pos)
{
	struct scull_buffer *dev = filp->private_data;
	int result;

	if (down_interruptible(&dev->sem))
	{
		return -ERESTARTSYS;
	}

	result = scullb_getwritespace(dev, filp);
	if (result)
	{
		return result;
	}
	printk("scull buffer: space is available to write to...\n");
	count = min(count, (size_t)spacefree(dev));
	if (dev->wp >= dev->rp)
	{
		count = min(count, (size_t)(dev->buffer_end - dev->wp));
	}
	else
	{
		count = min(count, (size_t)(dev->rp - dev->wp - 1));
	}
	/*printk("scullbuffer: accepting %li bytes to %p from %p\n", (long)count, dev->wp, buf);
	if (copy_from_user(dev->wp, buf, count))
	{
		up (&dev->sem);
		return -EFAULT;
	}
	dev->wp += count;
	if (dev->wp == dev->end)
	{
		dev->wp = dev->buffer;
	}*/
	printk("(scull_p_write) dev->wp:%p ::: dev->rp:%p\n", dev->wp, dev->rp);
	up(&dev->sem);
	wake_up_interruptible(&dev->consumer_queue);
	printk("scullbuffer: leaving scull_p_write...\n");
	return count;
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

int scullb_init(void)
{
	int result = -1;
	dev_t dev = 0;
/*
 * Get a range of MINOR numbers to work with, asking for a dynamic
 * MAJOR unless directed otherwise at load time.
 */
	if (MAJOR_NUM) 
	{
		dev = MKDEV(MAJOR_NUM, MINOR_NUM);
		result = register_chrdev_region(dev, DEVICE_COUNT, DEVICE_NAME);
		} 
	else // TODO remove if leaving major hardcoded
	{
		result = alloc_chrdev_region(&dev, MINOR_NUM, DEVICE_COUNT, DEVICE_NAME);
		MAJOR_NUM = MAJOR(dev);
	}
	if (result < 0) 
	{
		printk(KERN_INFO "scullbuffer: can't get MAJOR %d\n", MAJOR_NUM);
		// TODO release call necessarry?
		return result;
	}
        /* 
	 * allocate the devices -- we can't have them static, as the number
	 * can be specified at load time
	 */
	
	scullb_devno = dev;
	scullb_device = kmalloc(sizeof(struct scull_buffer), GFP_KERNEL);
	if (scullb_device == NULL) 
	{
		unregister_chrdev_region(dev, DEVICE_COUNT);
		// TODO release call necessary?
		return -ENOMEM;
	}
	memset(scullb_device, 0, DEVICE_COUNT * sizeof(struct scull_buffer));

	if (scullb_device->buffer == NULL)	// then the buffer hasn't been created, so create it.
	{
		scullb_device->buffer = kmalloc(BUF_SIZE, GFP_KERNEL);
                if (scullb_device->buffer == NULL)	// then memory was unsuccessfully allocated.
		{
			unregister_chrdev_region(dev, DEVICE_COUNT);
                        return -ENOMEM;
                }
		printk(KERN_INFO "scullbuffer: Buffer has been allocated %zu bytes.\n", ksize(scullb_device->buffer));
	}
	
	// Initialize device.
	scullb_device->buffer_size = sizeof(scullb_device->buffer);
	scullb_device->buffer_end = scullb_device->buffer + scullb_device->buffer_size;
	scullb_device->rp = scullb_device->wp = scullb_device->buffer;
	init_waitqueue_head(&scullb_device->producer_queue);
	init_waitqueue_head(&scullb_device->consumer_queue);
	scullb_device->num_producers = scullb_device->num_consumers = 0;
	init_MUTEX(&scullb_device->sem);
	scullb_setup_cdev(scullb_device);
	/*printk(KERN_INFO "scullbuffer: buffer_size: %zu\n", ksize(scullb_device->buffer_size));
	printk(KERN_INFO "scullbuffer: producers, consumers: %d,%d", scullb_device->num_producers, scullb_device->num_consumers);*/
	printk(KERN_INFO "scullbuffer: Hola (Module loaded).\n");
	return 0; /* succeed */
}

void scullb_cleanup(void)
{
	/*if (scullb_device == NULL)
	{*/
		cdev_del(&scullb_device->cdev);
	/*}*/
	kfree(scullb_device->buffer);
	kfree(scullb_device);
	unregister_chrdev_region(scullb_devno, DEVICE_COUNT);
	scullb_device = NULL;
	printk(KERN_INFO "scullbuffer: Peace (Module unloaded).\n");
}

module_init(scullb_init);
module_exit(scullb_cleanup);	


