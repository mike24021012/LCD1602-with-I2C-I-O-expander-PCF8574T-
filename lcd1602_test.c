#include <stdio.h>
#include <stdlib.h>	//system
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <sys/ioctl.h>
#include<pthread.h>

#define DEVICE_NAME "/dev/lcd1602"
#define MAGIC_NUMBER 'X'
#define IOCTL_SET	_IO(MAGIC_NUMBER,  1)

int main(void)
{
	int res,lcd1602_file;

	lcd1602_file = open(DEVICE_NAME, O_RDWR);
	if(lcd1602_file < 0)
	{
		perror("can not open sg90 device");
		exit(1);
	}
	
	while(1)
	{
	    printf("Input command, 1:run positive 2:run negative 3:stop 4:leave program.\n");
		scanf("%c",&mycmd);
		getchar();
		switch(mycmd)
	
	
	}
	
	if (lcd1602_file >= 0)	 //close humidityfd if open
	{
		close(lcd1602_file);
		printf("close file success.\n");
	}
	
	return 0;
}
