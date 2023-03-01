#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <asm/uaccess.h>
#include <linux/kfifo.h>
#include <linux/slab.h>

#define CHR_DEV_NAME "MyBufferedMem"
#define CHR_DEV_MAJOR 255

struct kfifo fifo;

static unsigned int N = 1;
static unsigned int M = 1;

module_param(N, int, 0000);
MODULE_PARM_DESC(N, "Inner Buffer Size");
module_param(M, int, 0000);
MODULE_PARM_DESC(M, "Read Size");


int chr_open(struct inode * inode, struct file * filep)
{
	int number = MINOR(inode->i_rdev);
	printk("MyBufferedMem Device Open : Minor Number is %d\n", number);
	return 0;
}

ssize_t chr_write(struct file * filep, const char * buf, size_t count, loff_t * f_pos)
{
	char val;
	int length = 0;

	while(count > 0)
	{
		if (kfifo_is_full(&fifo))
		{
			kfifo_out(&fifo, &val, sizeof(val));
			printk("fifo is full, out value: %c",val);
		}
		get_user(val, buf);
		kfifo_in(&fifo, &val, sizeof(char));
		count--;
		length++;
		buf++;
		printk("fifo in val: %c",val);
	}
	printk("write data\n");
	return length;
}

ssize_t chr_read(struct file * filep, char * buf, size_t count, loff_t * f_pos)
{
	ssize_t retval;
	char val = 'c';
	int length = M;

	if (*f_pos > 0)
	{
		retval = 0;
		goto out;
	}

	if (kfifo_is_empty(&fifo))
	{
		printk("kfifo empty\n");
		return 0;
	}
	
	retval = length;
	while (length > 0 && !kfifo_is_empty(&fifo))
	{
		kfifo_out(&fifo, &val, sizeof(val));
		put_user(val, buf);
		buf++;
		length--;
		printk("read data: %c",val);
	}
	printk("read length: %zd", retval);
	printk("complete reading data\n");
	*f_pos += retval;

out:
	return retval;
}

long chr_ioctl(struct file * filep, unsigned int cmd, unsigned long arg)
{
	int ret = 0;
	switch(cmd) {
		case 0: {
		char val = 'c';
		char* tmp = (char *)kmalloc(sizeof(char) * N, GFP_KERNEL);
		int i = 0;
		int j = 0;
		N = arg;

		while (!kfifo_is_empty(&fifo))
		{
			kfifo_out(&fifo, &val, sizeof(val));
			tmp[i] = val;
			printk("tmp value: %c",tmp[i]);
			i++;
		}

		kfifo_free(&fifo);
		ret = kfifo_alloc(&fifo, N, GFP_KERNEL);
		printk("kfifo realloc");

		while (j < i)
		{

			if (kfifo_is_full(&fifo))
				kfifo_out(&fifo, &val, sizeof(char));
			
			val = tmp[j];
			kfifo_in(&fifo, &val, sizeof(char));
			j++;
			printk("kfifo_in!");
		}
		
		kfree(tmp);
	
		if (ret)
			printk(KERN_ERR "error kfifo_alloc\n");
		break;
		}

		case 1: M = arg; break;
	}
	printk("ioctl N:%d M:%d\n", N, M);
	return 0;
}

int chr_release(struct inode * inode, struct file * filep)
{
	printk("MyBufferedMem Release\n");
	return 0;
}

struct file_operations chr_fops =
{
	.owner	= THIS_MODULE,
	.write	= chr_write,
	.read	= chr_read,
	.open	= chr_open,
	.release	= chr_release,
	.unlocked_ioctl	= chr_ioctl,
};

int chr_init(void)
{
	int registration;
	int ret;
	printk("MyBufferedMem(201611836,201713872) device driver registered\n");
	printk("N:%d M:%d", N,M);
	registration = register_chrdev(CHR_DEV_MAJOR, CHR_DEV_NAME, &chr_fops);
	if (registration < 0)
		return registration;

	ret = kfifo_alloc(&fifo, N, GFP_KERNEL);
	if (ret)
		printk(KERN_ERR "error kfifo_alloc\n");
	
	printk("Major Number : %d\n", registration);
	return 0;
}

void chr_exit(void)
{
	kfifo_free(&fifo);
	printk("MyBufferedMem(201611836,201713872) device driver clean up\n");
	unregister_chrdev(CHR_DEV_MAJOR, CHR_DEV_NAME);
}

module_init(chr_init);
module_exit(chr_exit);

MODULE_LICENSE("GPL");

