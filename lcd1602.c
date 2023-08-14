#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/i2c.h>
#include <linux/input.h>
#include <linux/delay.h>
#include <linux/gpio.h>
#include <linux/slab.h>
#include <linux/platform_device.h>
#include <linux/fs.h>
#include <linux/types.h>
#include <linux/init.h>
#include <mach/gpio.h>
#include <plat/gpio-cfg.h>
#include <asm/uaccess.h>

#include <linux/gpio.h>
#include <plat/iic.h>
#include <plat/gpio-cfg.h>
#include <plat/cpu.h>

#define LCD1602_NAME "lcd1602"
#define LCD1602_MAJOR 187


static ssize_t lcd1602_read(struct file *filp, char *buffer, size_t length, loff_t *offset){
   
    return 0;
}
static ssize_t lcd1602_write(struct file *filp, char *buffer, size_t length, loff_t *offset){
	return 0;
}
static int lcd1602_open(struct inode *inode, struct file *file){
    return 0;
}
static int lcd1602_release(struct inode *inode, struct file *file){
    return 0;
}

static struct file_operations fops = {
    .read       = lcd1602_read,
    .write      = lcd1602_write,
    .open       = lcd1602_open,
    .release    = lcd1602_release
};

static struct i2c_client *this_client;
static struct class *lcd1602_class;

static int lcd1602_probe(struct i2c_client *client, const struct i2c_device_id *id){
    printk("lcd1602_probe Called!!\n");
    this_client = client;

    int res;
    res = register_chrdev(LCD1602_MAJOR, LCD1602_NAME, &fops);
    if(res < 0){
        printk("register_chrdev() fail!!\n");
        return -1;
    }
    lcd1602_class = class_create(THIS_MODULE, "lcd1602");
    if(IS_ERR(lcd1602_class)){
        printk("class_create() fail!!\n");
        return -1;
    }
    device_create(lcd1602_class, NULL, MKDEV(LCD1602_MAJOR, 0), NULL, LCD1602_NAME);

    return 0;
}

static int __devexit lcd1602_remove(struct i2c_client *client){
    i2c_set_clientdata(client, NULL);
    device_destroy(lcd1602_class, MKDEV(LCD1602_MAJOR, 0));
    class_destroy(lcd1602_class);
    unregister_chrdev(LCD1602_MAJOR, LCD1602_NAME);

    printk("lcd1602_remove Called!!\n");
    return 0;
}


static const struct i2c_device_id lcd1602_id[] = {
    { "lcd1602", 0 },
    { }
};

static struct i2c_driver lcd1602_driver = {
    .probe = lcd1602_probe,
    .remove = __devexit_p(lcd1602_remove),
    .id_table = lcd1602_id,
    .driver = {
        		.name = LCD1602_NAME,
        		.owner = THIS_MODULE,
    		  },
};

static int __init lcd1602_init(void){
    return i2c_add_driver(&lcd1602_driver);
}
static void __exit lcd1602_exit(void){
    i2c_del_driver(&lcd1602_driver);
}


late_initcall(lcd1602_init);
module_exit(lcd1602_exit);

MODULE_AUTHOR("MEME48_JHH");
MODULE_DESCRIPTION("LCD1602 I2C");
MODULE_LICENSE("GPL");
