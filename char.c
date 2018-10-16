#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <asm/uaccess.h>
#include <linux/ioport.h>
#include <linux/io.h>

#define SIZE 1024

struct resource * led_res;
void __iomem *GPIOCOUT_VA;  //0x00
void __iomem *GPIOCOUTENB_VA; //0x04
void __iomem *GPIOCALTFN0_VA;//0x20
void __iomem *GPIOCALTFN1_VA;//0x24

void __iomem *GPIOEOUT_VA;  //0x00
void __iomem *GPIOEOUTENB_VA; //0x04
void __iomem *GPIOEALTFN0_VA;//0x20

int major = 0;
char KBuf[SIZE];

int my_open(struct inode *inode, struct file *filp)
{
	printk("my_open\n");

	return 0;  //success return 
}

int my_release(struct inode *inode, struct file *filp)
{
	printk("my_release\n");

	return 0;  //success return 
}

int led_get(char *GetValue)
{
	GetValue[0]  =   ! ((*(unsigned int *)GPIOEOUT_VA  >> 13) & 0x1) + 48;
	GetValue[1]  =    !(!!(*(unsigned int *)GPIOCOUT_VA   &  0x1 << 17))  + 48;
	GetValue[2]  =   ! ((*(unsigned int *)GPIOCOUT_VA  >> 8) & 0x1) + 48;
	GetValue[3]  = ! ( !!(*(unsigned int *)GPIOCOUT_VA   &  0x1 << 7))  + 48;

	printk(KERN_ALERT "GetValue : %s\n",GetValue);

	return 0;
}

int led_set(char *SetValue)
{

	char   led1Status =  SetValue[0] - 48;
	char   led2Status =  SetValue[1] - 48;
	char   led3Status =  SetValue[2] - 48;
	char   led4Status =  SetValue[3] - 48;
  

	*(unsigned int *)GPIOEOUT_VA   = (*(unsigned int *)GPIOEOUT_VA  &  ~(0x1 << 13)  ) | led1Status << 13 ;
	*(unsigned int *)GPIOCOUT_VA   = (*(unsigned int *)GPIOCOUT_VA  &  ~(0x1 << 17)  ) | led2Status << 17 ;
	*(unsigned int *)GPIOCOUT_VA   = (*(unsigned int *)GPIOCOUT_VA  &  ~(0x1 << 8)  ) |  led3Status << 8 ;
	*(unsigned int *)GPIOCOUT_VA   = (*(unsigned int *)GPIOCOUT_VA  &  ~(0x1 << 7)  ) |  led4Status << 7 ;


	return 0;
}

ssize_t my_read(struct file *filp, char __user *buf, size_t count, loff_t *offset)
{
	int ret;
	printk("my_read\n");
	printk("read user space count is %d\n",count);
	led_get(KBuf);
	
	if(count <= SIZE){
		ret = copy_to_user(buf,KBuf,count);
		ret = count;
	}else{
		printk("count > SIZE can not copy to user\n");
		return -1;
	}

	
	
	
	return ret;
}

ssize_t my_write(struct file *filp, const char __user *buf, size_t count, loff_t *offset)
{
	int ret = 0;
	printk("my_write\n");
	printk("write from user space count is %d\n",count);
	
	if(count <= SIZE){
		ret = copy_from_user(KBuf,buf,count);
		if(ret > 0){
			printk("copy_from_user(KBuf,buf,count) failed\n");
			return -1;
		}else{
			printk("the string is %s\n",KBuf);	
			ret =count;
			led_set(KBuf);		
		}
	}else{
		printk("count > size, can not copy_from_user\n");
		return -1;
	}   
	
	
	return ret;
}


struct file_operations bin_fops={

	.open = my_open,
	.release = my_release,
	.read = my_read,
	.write = my_write,

};



int char_test_init(void)
{
	printk("char_test_init_bin 10/15 16:43\n");
	
	//【1】申请GPIO内存资源
	led_res = request_mem_region(0xC001C000, 0x3000, "gec6818_led_res");
	if(led_res == NULL){
			printk(" request_mem_region Failed!\n");
			return -EINVAL;
	}else{
		printk("request men region success\n");
	}

	 //【2】对申请成功的GPIO地址，进行内存映射，获得对应的虚拟地址
	GPIOCOUT_VA = ioremap(0xC001C000, 0x3000);
	GPIOCOUTENB_VA = GPIOCOUT_VA + 0x04;
	GPIOCALTFN0_VA = GPIOCOUT_VA + 0x20;
	GPIOCALTFN1_VA = GPIOCOUT_VA + 0x24;
	
	GPIOEOUT_VA = GPIOCOUT_VA + 0x2000;
	GPIOEOUTENB_VA = GPIOEOUT_VA + 0x04;
	GPIOEALTFN0_VA = GPIOEOUT_VA + 0x20;
//	GPIOEALTFN1_VA = GPIOEOUT_VA + 0x24;


     //将引脚初始化成function1--GPIOC
	*(unsigned int *)GPIOCALTFN0_VA &= ~(3<<26);   //GPIOE13-->func0->GPIO
	 
	*(unsigned int *)GPIOCALTFN1_VA &= ~(3<<2); 
	*(unsigned int *)GPIOCALTFN1_VA |= (1<<2);    //GPIOC17-->func1->GPIO
	
	*(unsigned int *)GPIOCALTFN0_VA &= ~(3<<16);   //GPIOC8-->func0->GPIO
	*(unsigned int *)GPIOCALTFN0_VA |= (1<<16);
	
	*(unsigned int *)GPIOCALTFN0_VA &= ~(3<<14);   //GPIOC7-->func0->GPIO
	*(unsigned int *)GPIOCALTFN0_VA |= (1<<14);
	
	*(unsigned int *)GPIOCALTFN0_VA &= ~(3<<28);   //GPIOC14-->func0->GPIO
	*(unsigned int *)GPIOCALTFN0_VA |= (1<<28);    //beep
	

	//将CPIOC设置为OUTPUT
	*(unsigned int *)GPIOCOUTENB_VA |= (1<<17) | (1<<7) | (1<<8) ;   //led
	*(unsigned int *)GPIOEOUTENB_VA |= (1<<13);
	*(unsigned int *)GPIOCOUTENB_VA |= (1<<14);   //beep

    //  设置GPIOC output 1，LED BEEP -->off
	*(unsigned int *)GPIOCOUT_VA |= (1<<17) | (1<<7) | (1<<8);	  //led
	*(unsigned int *)GPIOEOUT_VA |= (1<<13);
	*(unsigned int *)GPIOCOUT_VA &= ~(1<<14);	  //beep
	
	//注册字符设备驱动
	major = register_chrdev(0,             //主设备号 0~255
	 					   "bin_char",     //设备驱动名称
	 				       &bin_fops       //文件操作接口
	 				       );

	if(major < 0)
		printk("register_chrdev failed\n");
	else
		printk("register_chrdev of major is %d\n",major);
						 
	return 0;
}

void char_test_exit(void)
{
	printk("char_test_exit\n");
	
	iounmap(GPIOCOUT_VA);
	release_mem_region(0xC001C000, 0x3000);
	
	unregister_chrdev(major,    //主设备号
					  "bin_char"  //设备驱动名称
					  );
}


module_init(char_test_init);
module_exit(char_test_exit);


MODULE_LICENSE("GPL");