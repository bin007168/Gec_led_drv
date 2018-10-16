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

int main(int argc,char *argv[])
{
	int ret,sn;
	char wbuf[50];
	char rbuf[50];
	
	fd = open("/dev/char_test",O_RDWR);
	if(fd < 0){
		printf("open failed\n");
		return -1;
	}else{
		printf("open fd success 13:10\n");
	}
	
	if(argc != 2){
		printf("usage: ./char_test  0000 (0 0 0 0 means 1->2->3->4 led is on)\n");
		return -1;
	}
	
    strcpy(wbuf,argv[1]);
	ret = write(fd,wbuf,strlen(wbuf));
	printf("write count of ret to kernel is %d\n",ret);
	
	ret = read(fd,rbuf,20);
	printf("read count of ret from kernel is %d \n",ret);
	printf("the data from kernel is [%s]  read the count is %d\n",rbuf,ret);

	close(fd);

	return 0;
}



