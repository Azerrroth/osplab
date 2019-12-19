#include<stdio.h>
#include<stdlib.h>
#include<pthread.h>
#include<fcntl.h>
#include<sys/mman.h>
#include<string.h>
#include<errno.h>
#include<unistd.h>
typedef struct Info
{
	char fromFile[255];	//源文件
	char toFile[255];	//目标文件
	int num;	//第几个线程
	int legth;	//长度
	int offset;	//偏移
	int filesize;	//待拷文件的大小
}Info;

void* doThread(void *arg)
{
	Info *info = (Info*)arg;

	int fin= open(info->fromFile,O_RDONLY);
	int fout = open(info->toFile,O_RDWR);

	printf("这是第%d线程,开始拷贝\n",info->num);
	//每个线程里面，我都把fromFile和toFile的filesize大小映射一下
	char *s_addr=(char *)mmap(NULL,info->filesize,PROT_READ,MAP_SHARED,fin,0);
	char *d_addr=(char *)mmap(NULL,info->filesize,PROT_READ|PROT_WRITE,MAP_SHARED,fout,0);

	//每个线程负责拷贝文件的不同部分在这里就体现出来了，拷贝的长度通过参数得知
	memcpy(d_addr+info->offset,s_addr+info->offset,info->legth);
	printf("第%d个线程拷贝结束\n",info->num);

	munmap(d_addr,info->legth);//解除映射
	munmap(s_addr,info->legth);
	return NULL;
}
 
int main(int argc,char *argv[])
{
	pthread_t *tid;
	Info *info;
	info=(Info *)malloc(5*sizeof(Info));
	tid=(pthread_t *)malloc(5*sizeof(pthread_t));
	int fd=open(argv[1],O_RDONLY);
	if(fd<0)
	{
		perror("open src failed:");
		exit(errno);
	}
	umask(000);
	int fd2=open(argv[2],O_RDWR|O_CREAT,0777);
	if(fd2<0)
	{
	perror("open des failed:");
	exit(errno);
	}
	int filesize=lseek(fd,0,SEEK_END);
	ftruncate(fd2,filesize);//改变文件大小，也可以用lseek，然后write一下实现，ftruncate更简单些

	int offset=filesize/5;
	for(int i=0;i<4;++i)
	{
		//前四个线程的legth都是offset，第五个线程的legth是filesize-4*offset
		info[i].num=i;//我自己给线程起的编号，从0-4
		strcpy(info[i].fromFile,argv[1]);
		strcpy(info[i].toFile,argv[2]);
		info[i].legth=offset;
		info[i].offset=i*offset;
		info[i].filesize=filesize;
	}
	//第五个线程的legth和前四个不一样
	info[4].num=4;
	strcpy(info[4].fromFile,argv[1]);
	strcpy(info[4].toFile,argv[2]);
	info[4].legth=filesize-4*offset;
	printf("%d",info[4].legth);
	info[4].offset=4*offset;
	info[4].filesize=filesize;
	for(int i=0;i<5;++i)//创建五个线程来实现文件的拷贝
	{
		pthread_create(&tid[i],NULL,doThread,&info[i]);
	}
	for(int j=0;j<5;++j)//主线程回收创建的五个线程
	{
		pthread_join(tid[j],NULL);
	}
	return 0;
}
