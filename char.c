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

int major = 0;
char KBuf[SIZE];

int my_open(struct inode *inode, struct file *filp)
{
//	printk("my_open\n");

	return 0;  //success return 
}

int my_release(struct inode *inode, struct file *filp)
{
//	printk("my_release\n");

	return 0;  //success return 
}

ssize_t my_read(struct file *filp, char __user *buf, size_t count, loff_t *offset)
{
	int ret;
//	printk("my_read\n");
//	printk("read user space count is %d\n",count);
	char ToUserBuf[40]= "hello I am kernel data";
	
	ret = copy_to_user(buf,ToUserBuf,count);
	
	
	return ret;
}

ssize_t my_write(struct file *filp, const char __user *buf, size_t count, loff_t *offset)
{
	int ret = 0;
	int recv;
	printk("my_write\n");
//	printk("write from user space count is %d\n",count);
	
	if(count <= SIZE){
		ret = copy_from_user(KBuf,buf,count);
		if(ret > 0){
			printk("copy_from_user(KBuf,buf,count) failed\n");
			return -1;
		}else{
			printk("copy_from_user(KBuf,buf,count) success\n");
			printk("the string is %s\n",KBuf);
			//  设置GPIOC output 1，LED -->off
			recv = (unsigned int)(KBuf[0]);
			printk("the recv num is %d\n",recv);
			if(recv == 48){
				*(unsigned int *)GPIOCOUT_VA &= ~(1<<17);
			}else if(recv == 49){
				*(unsigned int *)GPIOCOUT_VA |= (1<<17);
			}else if(recv == 50){
				*(unsigned int *)GPIOCOUT_VA |= (1<<14);
			}else if(recv == 51){
				*(unsigned int *)GPIOCOUT_VA &= ~(1<<14);
			}
		
			ret =count;
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
	led_res = request_mem_region(0xC001C000, 0x1000, "gec6818_led_res");
	if(led_res == NULL){
			printk(" request_mem_region Failed!\n");
			return -EINVAL;
	}else{
		printk("request men region success\n");
	}

	 //【2】对申请成功的GPIO地址，进行内存映射，获得对应的虚拟地址
	GPIOCOUT_VA = ioremap(0xC001C000, 0x1000);
	GPIOCOUTENB_VA = GPIOCOUT_VA + 0x04;
	GPIOCALTFN0_VA = GPIOCOUT_VA + 0x20;
	GPIOCALTFN1_VA = GPIOCOUT_VA + 0x24;


     //将引脚初始化成function1--GPIOC
	*(unsigned int *)GPIOCALTFN1_VA &= ~(3<<2); 
	*(unsigned int *)GPIOCALTFN1_VA |= (1<<2);    //GPIOC17-->func1->GPIO

	//将CPIOC设置为OUTPUT
	*(unsigned int *)GPIOCOUTENB_VA |= (1<<17);   //led
	*(unsigned int *)GPIOCOUTENB_VA |= (1<<14);   //beep

    //  设置GPIOC output 1，LED BEEP -->off
	*(unsigned int *)GPIOCOUT_VA |= (1<<17);	  //led
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
	release_mem_region(0xC001C000, 0x1000);
	
	unregister_chrdev(major,    //主设备号
					  "bin_char"  //设备驱动名称
					  );
}


module_init(char_test_init);
module_exit(char_test_exit);


MODULE_LICENSE("GPL");