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

// commands
#define LCD_CLEARDISPLAY 0x01
#define LCD_RETURNHOME 0x02
#define LCD_ENTRYMODESET 0x04
#define LCD_DISPLAYCONTROL 0x08
#define LCD_CURSORSHIFT 0x10
#define LCD_FUNCTIONSET 0x20
#define LCD_SETCGRAMADDR 0x40
#define LCD_SETDDRAMADDR 0x80

// flags for display on/off control
#define LCD_DISPLAYON 0x04
#define LCD_DISPLAYOFF 0x00
#define LCD_CURSORON 0x02
#define LCD_CURSOROFF 0x00
#define LCD_BLINKON 0x01
#define LCD_BLINKOFF 0x00

// flags for display/cursor shift
#define LCD_DISPLAYMOVE 0x08
#define LCD_CURSORMOVE 0x00
#define LCD_MOVERIGHT 0x04
#define LCD_MOVELEFT 0x00

// flags for display entry mode
#define LCD_ENTRYRIGHT 0x00
#define LCD_ENTRYLEFT 0x02
#define LCD_ENTRYSHIFTINCREMENT 0x01
#define LCD_ENTRYSHIFTDECREMENT 0x00

// flags for function set
#define LCD_8BITMODE 0x10
#define LCD_4BITMODE 0x00
#define LCD_2LINE 0x08
#define LCD_1LINE 0x00
#define LCD_5x10DOTS 0x04
#define LCD_5x8DOTS 0x00

// flags for backlight control
#define LCD_BACKLIGHT 0x08
#define LCD_NOBACKLIGHT 0x00

#define En 0b00000100  // Enable bit
#define Rw 0b00000010  // Read/Write bit
#define Rs 0b00000001  // Register select bit


//lcd1602 character driver & i2c driver
#define LCD1602_NAME "lcd1602"
#define LCD1602_MAJOR 187
static struct i2c_client *this_client;
static struct class *lcd1602_class;

static u8 my_command;
static u8 mode;

static int lcd1602_send(const struct i2c_client *client, u8 value, u8 mode){

	u8 highnib = value & 0xf0;
	u8 lownib = (value << 4) & 0xf0;
	s32 result;
	
	result = i2c_smbus_write_byte(client, highnib | mode | En);
	if(result < 0)
		return result;
	result = i2c_smbus_write_byte(client, highnib | mode);
	if(result < 0)
		return result;
	
	result = i2c_smbus_write_byte(client, lownib | mode | En);
	if(result < 0)
		return result;
	result = i2c_smbus_write_byte(client, lownib | mode);
	if(result < 0)
		return result;
	
	return 0;
}

static int command(const struct i2c_client *client, u8 value){
	mode = 0;
	mode |= LCD_BACKLIGHT;
	//printk("command: %d\n", value);
	return lcd1602_send(client,value,mode);
}

static int write(const struct i2c_client *client, u8 value){
	mode = 0;
	mode |= LCD_BACKLIGHT;
	mode |= Rs;
	return lcd1602_send(client,value,mode);
}

static int read(const struct i2c_client *client, u8 value){
	mode = 0;
	mode |= LCD_BACKLIGHT;
	mode |= Rs;
	mode |= Rw;
	return lcd1602_send(client,value,mode);
}

static int cursor_return_home(void){
	s32 result;
	
	my_command = 0;
	my_command |= LCD_CLEARDISPLAY;

	result = command(this_client, my_command);
	if(result < 0)
		return result;
	mdelay(5);
	
	return 0;
}

static int display_clear(void){
	s32 result;
	
	my_command = 0;
	my_command |= LCD_RETURNHOME;

	result = command(this_client, my_command);
	if(result < 0)
		return result;
	mdelay(5);
	
	return 0;
}

static int cursor_move(void){
	s32 result;

	my_command = 0;
	my_command |= LCD_CURSORSHIFT;
	my_command |= LCD_CURSORMOVE;
	my_command |= LCD_MOVERIGHT;

	result = command(this_client, my_command);
	if(result < 0)
		return result;
	return 0;
}

static int display_setting(void){
	s32 result;

	my_command = 0;
	my_command |= LCD_DISPLAYCONTROL;
	my_command |= LCD_DISPLAYON;
	my_command |= LCD_CURSORON;
	my_command |= LCD_BLINKON;

	result = command(this_client, my_command);
	if(result < 0)
		return result;
	return 0;
}

static int initial_setting(void){

	mdelay(50);
	
	// Now we pull RS and R/W low to begin commands
	// reset expanderand turn backlight off
	my_command = 0;
	u8 highnib = my_command & 0xf0;
	u8 lownib = (my_command << 4) & 0xf0;
	u8 nobacklight_mode = 0;
	s32 result;
	
	result = i2c_smbus_write_byte(this_client, highnib | nobacklight_mode | En);
	if(result < 0)
		return result;
	result = i2c_smbus_write_byte(this_client, highnib | nobacklight_mode);
	if(result < 0)
		return result;
	
	result = i2c_smbus_write_byte(this_client, lownib | nobacklight_mode | En);
	if(result < 0)
		return result;
	result = i2c_smbus_write_byte(this_client, lownib | nobacklight_mode);
	if(result < 0)
		return result;	
	mdelay(1000);

  	// put the LCD into 4 bit mode
	// this is according to the hitachi HD44780 datasheet
	// figure 24, pg 46
	
	// we start in 8bit mode, try to set 4 bit mode
    	result = command(this_client, 0x03 << 4);
	if(result < 0)
		return result;
   	mdelay(5); // wait min 4.1ms
   
    	// second try
   	result = command(this_client, 0x03 << 4);
	if(result < 0)
		return result;
 	mdelay(5); // wait min 4.1ms
   
    	// third go!
    	result = command(this_client, 0x03 << 4);
	if(result < 0)
		return result;
    	mdelay(5);
   
    	// finally, set to 4-bit interface
    	result = command(this_client, 0x02 << 4);
	if(result < 0)
		return result;
    	mdelay(5); 

	// set # lines font size etc
   	my_command = 0;
    	my_command |= LCD_FUNCTIONSET;
	my_command |= LCD_2LINE;
	my_command |= LCD_5x10DOTS;
	result = command(this_client, my_command);
	if(result < 0)
		return result;  
	
	// turn the display on with cursor and blinking default
	my_command = 0;
	my_command |= LCD_DISPLAYCONTROL;
	my_command |= LCD_DISPLAYON;
	my_command |= LCD_CURSORON;
	my_command |= LCD_BLINKON;
	result = command(this_client, my_command);
	if(result < 0)
		return result;
	
	// Initialize to default text direction (for roman languages)
	// set the entry mode
	my_command = 0;
	my_command |= LCD_ENTRYLEFT;
	my_command |= LCD_ENTRYMODESET;
	my_command |= LCD_ENTRYSHIFTDECREMENT;
	result = command(this_client, my_command);
	if(result < 0)
		return result;
	
	return 0;
}

static ssize_t lcd1602_read(struct file *filp, char *buffer, size_t length, loff_t *offset){
    return 0;
}

static ssize_t lcd1602_write(struct file *filp,const char *buffer, size_t length, loff_t *offset){

	s32 result;
	
	result = cursor_return_home();
	if(result < 0)
			return result;

	result = display_clear();
	if(result < 0)
			return result;
	
	int i=0;
	while(buffer[i]!='\0')
	{	
		if(i == 16)
		{

			result = command(this_client, LCD_SETCGRAMADDR | LCD_SETDDRAMADDR);
			if(result < 0)
				return result;
		}
		result = write(this_client, buffer[i]);
		if(result < 0)
			return result;
		++i;
	}

	return 0;
}

static int lcd1602_open(struct inode *inode, struct file *file){
	s32 result;

	result = initial_setting();
	if(result < 0)
		return result;
	
	printk("intialize lcd1602 ok\n");	
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
