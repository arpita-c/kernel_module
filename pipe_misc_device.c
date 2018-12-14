#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/semaphore.h>
#include <linux/miscdevice.h>
#include <linux/string.h>
#include <linux/stat.h>
#include <linux/slab.h>

#define SUCCESS 0
#define DEVICE_NAME "pipe_misc_device"
#define BUF_LEN 200

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Arpita Chakraborty");

static int pipe_open(struct inode *, struct file *);
static int pipe_release(struct inode *, struct file *);
static ssize_t pipe_read(struct file *, char *, size_t, loff_t *);
static ssize_t pipe_write(struct file *, const char *, size_t, loff_t *);


char** buffer_array_dev;

static int no_of_times = 0;
static int read_count = 0;
static int write_count = 0;
static int num_empty_buff;

int buff_len = 0;
module_param(buff_len, int, S_IRUSR | S_IWUSR);

static struct file_operations pipe_misc_device_fops = {
	.owner = THIS_MODULE,
	.read = pipe_read,
	.write = pipe_write,
	.open = pipe_open,
	.release = pipe_release
};

static struct miscdevice miscdev = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = DEVICE_NAME,
	.fops = &pipe_misc_device_fops
};



struct semaphore empty;
struct semaphore full;
struct semaphore mutex_read;
struct semaphore mutex_write;

/*
 * This function is called when the module is loaded.
 */

static int init_pipe_misc_device(void)
{
	int count = 0;
	int ret_val  = misc_register(&miscdev);

	if (ret_val < 0) 
	{
		printk(KERN_ALERT "Device failed with  %d\n", ret_val);
	  	return ret_val;
	}

	sema_init(&full, 0);
    sema_init(&empty, buff_len);
	sema_init(&mutex_read, 1);
	sema_init(&mutex_write, 1);

	buffer_array_dev = (char**)kzalloc(buff_len*sizeof(char*), GFP_KERNEL);
	for (count = 0; count < buff_len; count++)
	{
		buffer_array_dev[count] = (char*)kzalloc((BUF_LEN+1)*sizeof(char), GFP_KERNEL);
		buffer_array_dev[count][BUF_LEN] = '\0';
	}


    if (!buffer_array_dev)
    {
		printk(KERN_ALERT "***Error: Unable to allocate memory using kzalloc***\n");
		return -EINVAL;
    }

    num_empty_buff = buff_len;

    printk(KERN_INFO "####Hey,Device registered successfully!#####\n");

	return SUCCESS;
}

/*
 * This function is called when the module is unloaded/removed.
 */

void exit_pipe_misc_device(void)
{
	int count = 0;

	for (count = 0; count < buff_len; count++)
	{
		kfree(buffer_array_dev[count]);
	}
	kfree(buffer_array_dev);
	misc_deregister(&miscdev);
	printk(KERN_INFO "Bye \n");
}


static int pipe_open(struct inode *inode, struct file *file)
{
	no_of_times=no_of_times+1;
	printk(KERN_ALERT "MyPipe Opened %d no_of_times\n", no_of_times);
	return SUCCESS;
}

static int pipe_release(struct inode *inode, struct file *file)
{
	printk(KERN_ALERT "Pipe Closed!\n");
	no_of_times=no_of_times-1;
	return SUCCESS;
}

static ssize_t pipe_read(struct file *filp, char __user *buff, size_t len, loff_t *offset)
{

	unsigned int ret_val=0,buff_counter;
	
	if(down_interruptible(&full) < 0)
	{
		printk(KERN_INFO "bye bye user!\n");
	}
	if(down_interruptible(&mutex_read) < 0)
	{
		printk(KERN_INFO "bye bye user!\n");
	}

	read_count %= buff_len;

	for(buff_counter=0; buff_counter < len; buff_counter++)
	{
		ret_val = copy_to_user(&buff[buff_counter], &buffer_array_dev[read_count][buff_counter], 1);
		if(ret_val < 0)
		{
			printk(KERN_ALERT "Error in copy_to_user");
			return ret_val;
		}
	}

	read_count++;
	num_empty_buff++;
											
	up(&mutex_read);
	up(&empty);

	return buff_counter;
}

static ssize_t pipe_write(struct file *filp, const char *buff, size_t len, loff_t *offset)
{
	unsigned int ret_val=0;
	int buff_counter=0;

	if(down_interruptible(&empty) < 0)
	{
		printk(KERN_INFO "bye bye user!\n");
	}
	if(down_interruptible(&mutex_write) < 0)
	{
		printk(KERN_INFO "bye bye user!\n");
	}

	write_count %= buff_len;

	for(buff_counter=0; buff_counter < len; buff_counter++)
	{		
		ret_val = copy_from_user(&buffer_array_dev[write_count][buff_counter], &buff[buff_counter], 1);
		if(ret_val < 0)
		{
			printk(KERN_ALERT "Error in copy_to_user");
			return ret_val;
		}
	}
		
	write_count++;
	num_empty_buff--;
	
	up(&mutex_write);
	up(&full);
	
	return buff_counter;
}



module_init(init_pipe_misc_device);
module_exit(exit_pipe_misc_device);