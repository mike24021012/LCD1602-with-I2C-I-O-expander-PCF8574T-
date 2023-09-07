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
	int res,lcd1602_file,i;
	char my_command,my_buf[50],sentence[33];
	
	lcd1602_file = open(DEVICE_NAME, O_RDWR);
	if(lcd1602_file < 0)
	{
		perror("can not open lcd1602 device");
		exit(1);
	}
	printf("open lcd1602 device success.\n");

	while(1)
	{
		printf("Input command, 1:write sentences to lcd1602, 2:leave program.\n");
		scanf("%c",&my_command);
		getchar();
		switch(my_command)
		{
		case '1':
			printf("please input your sentences(max length: 32 characters):\n");
			scanf("%[^\n]s",my_buf);
			getchar();
			for(i=0;i<=31;++i)
				sentence[i]=my_buf[i];
			my_buf[32]='\0';
			res = write(lcd1602_file,my_buf,sizeof(my_buf));
    		if(res < 0)
    		{
    			printf("write err!\n");
			}
			continue;
		case '2':
			break;
		default:
			printf("Wrong command,please input again.\n");
			continue;
		}
		break;		
	}
	
	if (lcd1602_file >= 0)	 //close humidityfd if open
	{
		close(lcd1602_file);
		printf("close lcd1602 device success.\n");
	}
	
	return 0;
}
