#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/gpio.h>
#include <linux/string.h>
#include <asm/io.h>
#include <asm/uaccess.h>
#include <linux/signal.h>
#include <linux/interrupt.h>

#include <asm/mach-atheros/atheros.h>

static int major = 100;
static int minor = 100;
static dev_t devnum = 0;
static struct cdev mydev;
static char key = 0;
static int irq = 0;
static int test_value = 0;
static wait_queue_head_t mywait;

#define GPIOIRQ 	12

int ath_gpio_in_val(int gpio)
{
	return ((1 << gpio) & (ath_reg_rd(ATH_GPIO_IN)));
}

void ath_gpio_out_val(int gpio, int val)
{
	if (val & 0x1) {
		ath_reg_rmw_set(ATH_GPIO_OUT, (1 << gpio));
	} else {
		ath_reg_rmw_clear(ATH_GPIO_OUT, (1 << gpio));
	}
}


static void
ath_gpio_intr_enable(unsigned int irq)
{
	ath_reg_rmw_set(ATH_GPIO_INT_MASK,
				(1 << (irq - ATH_GPIO_IRQ_BASE)));
}

static void
ath_gpio_intr_disable(unsigned int irq)
{
	ath_reg_rmw_clear(ATH_GPIO_INT_MASK,
				(1 << (irq - ATH_GPIO_IRQ_BASE)));
}

void
ath_gpio_set_fn(int gpio, int fn)
{
#define gpio_fn_reg(x)	((x) / 4)
#define gpio_lsb(x)	(((x) % 4) * 8)
#define gpio_msb(x)	(gpio_lsb(x) + 7)
#define gpio_mask(x)	(0xffu << gpio_lsb(x))
#define gpio_set(x, f)	(((f) & 0xffu) << gpio_lsb(x))

	uint32_t *reg = ((uint32_t *)GPIO_OUT_FUNCTION0_ADDRESS) +
					gpio_fn_reg(gpio);

	ath_reg_wr(reg, (ath_reg_rd(reg) & ~gpio_mask(gpio)) | gpio_set(gpio, fn));
}

static int myopen (struct inode *inode, struct file *file)
{
	printk("my open !!!!\n");
	return 0;
}

static int myclose (struct inode *inode, struct file *file)
{
	printk("my close !!!\n");
	return 0;
}

static  ssize_t myread (struct file *file, char __user *buf, size_t len, loff_t *lof)
{
	int count = 0;
	key = 0;
    ath_gpio_out_val(14, test_value);
    ath_gpio_out_val(16, test_value);
	test_value = 0;
	wait_event_interruptible(mywait, key);// 阻塞
	copy_to_user(buf, &key, 1);

	printk("my read !\n");
	return count;
}

static ssize_t mywrite (struct file *file, const char __user *buf, size_t len, loff_t *lof)
{
	int count = -1;
	
	printf("mywrite\n");
	return count;
}

static struct file_operations myops = {
	.open = myopen,
	.release = myclose,
	.read = myread,
	.write = mywrite,
};

static irqreturn_t do_key(int  irqnum, void * dev)
{
	key = 'a';
	wake_up_interruptible(&mywait);//唤醒
    test_value = 1;
	printk(" irqnum = %d dev= %s\n", irqnum, (char *)dev);
	return IRQ_HANDLED;
}

static void gpio_init(void)
{
#if 1
    ath_reg_rmw_clear(ATH_GPIO_OE, 1 << 14);       //set the gpio14/16 output
    ath_gpio_set_fn(14, 0);
    ath_reg_rmw_clear(ATH_GPIO_OE, 1 << 16);
    ath_gpio_set_fn(16, 0);
#endif

    irq = ATH_GPIO_IRQn(GPIOIRQ);
  
          //set gpio12 intterupt high level
	ath_reg_rmw_set(ATH_GPIO_OE, 1 << GPIOIRQ); 
//    ath_reg_rmw_set(ATH_GPIO_INT_TYPE, (1 << 12));
//    ath_reg_rmw_set(ATH_GPIO_INT_POLARITY, (1 << 12));
	ath_reg_rmw_set(ATH_GPIO_INT_ENABLE, (1 << GPIOIRQ));  //have a bug
    ath_gpio_intr_enable(irq);
}

static __init int test_init(void) 
{
	int ret = -1;
  
	devnum = MKDEV(major, minor); 
	init_waitqueue_head(&mywait);
	ret = register_chrdev_region(devnum, 1, "mytest");
	if (ret)
		return -1;
	cdev_init(&mydev, &myops);
	ret = cdev_add(&mydev, devnum, 1); 
	if (ret)
		unregister_chrdev_region(devnum, 1);
    gpio_init();
    ret = request_irq(irq, do_key, IRQF_SHARED | IRQF_TRIGGER_HIGH, "key1", "key"); 
//    ret = request_irq(irq, do_key, 0, "key1", "key");
	if (ret)
    {
		cdev_del(&mydev);
        ath_gpio_intr_disable(irq);
        printk("request_irq gpio12 failed %d\n", ret);
    }
	printk("------------------------hello ap2x usb-----------------------------\n");
	return 0;
}

static __exit void test_exit(void)
{
	cdev_del(&mydev);
	unregister_chrdev_region(devnum, 1);
    ath_gpio_intr_disable(irq);
	free_irq(irq, "key");
	printk("------------------------byebye ap2x usb-----------------------------\n");
}

module_init(test_init);
module_exit(test_exit);
MODULE_LICENSE("GPL");
