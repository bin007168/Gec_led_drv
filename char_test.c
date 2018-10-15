#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>

#define LED_ON   0
#define LED_OFF  1

#define BEEP_ON  3
#define BEEP_OFF 4

int fd;  //文件描述符


int led_set(int status,int sn)
{
	switch(sn)
	{
		case 1:
			if(status == 3){
				write(fd,"2",1);
			}else{
				write(fd,"3",1);
			}
			break;
			
		case 2:
			if(status == 0){
				write(fd,"0",1);
			}else{
				write(fd,"1",1);
			}
			break;
		
		default:
			break;
		
	}
}


int main(int argc,char *argv[])
{
	int ret,sn;
	char buf[50];
	
	fd = open("/dev/test_char",O_RDWR);
	if(fd < 0){
		printf("open failed\n");
		return -1;
	}else{
		printf("open fd success 11:50\n");
	}
	
	if(argc != 3){
		printf("usage: ./char_test  beep/led2  on_led/off_led on_beep/off_beep\n");
		return -1;
	}
	
	if(strcmp(argv[1],"led2") == 0){
		sn = 2;
	}else if(strcmp(argv[1],"beep") == 0){
		sn = 1;
	}else{
		printf("usage: ./test_char  beep/led2  on_led/off_led on_beep/off_beep\n");
		return -1;
	}
	
	if(strcmp(argv[2],"on_led") == 0){
		led_set(LED_ON,sn);
	}else if(strcmp(argv[2],"off_led") == 0){
		led_set(LED_OFF,sn);
	}else if(strcmp(argv[2],"on_beep") == 0){
		led_set(BEEP_ON,sn);
	}else if(strcmp(argv[2],"off_beep") == 0){
		led_set(BEEP_OFF,sn);
	}else{
		printf("usage: ./test_char  beep/led2  on_led/off_led on_beep/off_beep\n");
		return -1;
	}

	
	ret = read(fd,buf,40);
	printf("read count of ret from kernel is %d \n",ret);
	printf("the data from kernel is %s \n",buf);

/*	memset(buf,0,sizeof(buf));
    strcpy(buf,"hello dear girl");
	ret = write(fd,buf,strlen(buf));
	printf("write count of ret to kernel is %d\n",ret);
*/
	close(fd);

	return 0;
}



