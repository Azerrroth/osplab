#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>

typedef struct Info
{
	char fromFile[255];	//源文件
	char toFile[255];	//目标文件
	int num;	//第几个线程
	int legth;	//长度
	int offset;	//偏移
	int filesize;	//待拷文件的大小
}Info;

int is_dir(char* path)
{
	struct stat pa;
	stat(path, &pa);
	if(S_ISDIR(pa.st_mode))	{
		return 1;
	}
	else {
		return 0;
	}
}

int endwith(char* s,char c) // 用于判断字符串结尾是否为“/”
{
	if(s[strlen(s)-1]==c){
		return 1;
	}
	else{
		return 0;
	}
}

void* doThread(void *arg)
{
	Info *info = (Info*)arg;

	int fin= open(info->fromFile,O_RDONLY);
	int fout = open(info->toFile,O_RDWR);

	// printf("这是第%d线程,开始拷贝\n",info->num);
	//每个线程里面，我都把fromFile和toFile的filesize大小映射一下
	char *s_addr=(char *)mmap(NULL,info->filesize,PROT_READ,MAP_SHARED,fin,0);
	char *d_addr=(char *)mmap(NULL,info->filesize,PROT_READ|PROT_WRITE,MAP_SHARED,fout,0);

	//每个线程负责拷贝文件的不同部分在这里就体现出来了，拷贝的长度通过参数得知
	memcpy(d_addr+info->offset,s_addr+info->offset,info->legth);
	// printf("第%d个线程拷贝结束\n",info->num);

	munmap(d_addr,info->legth); // 解除映射
	munmap(s_addr,info->legth);
	return NULL;
}

void copy_file_multithreads(char* source, char* destination, int num)
{
	pthread_t *tid;
	Info *info;
	info = (Info *)malloc(num * sizeof(Info));
	tid = (pthread_t *)malloc(num * sizeof(pthread_t));
	int fd_src = open(source, O_RDONLY);
	if(fd_src < 0)
	{
		perror("Open source failed");
		exit(errno);
	}
	umask(000);
	int fd_dest = open(destination, O_RDWR|O_CREAT, 0777);
	if(fd_dest < 0)
	{
		perror("Open destination failed");
		exit(errno);
	}
	int file_size = lseek(fd_src, 0, SEEK_END); // 获取文件尾得读写位置来获取文件大小
	ftruncate(fd_dest,file_size);
	
	int offset= file_size / num;
	for(int i = 0; i < num-1; ++i)
	{
		// 前number-1个线程的legth都是offset，第number个线程的legth是filesize-number-1*offset
		info[i].num = i; // 我自己给线程起的编号，从0~number-1
		strcpy(info[i].fromFile,source);
		strcpy(info[i].toFile,destination);
		info[i].legth = offset;
		info[i].offset = i * offset;
		info[i].filesize = file_size;
	}
	// 第number个线程的length和前number-1个不一样
	info[num-1].num=num-1;
	strcpy(info[num-1].fromFile,source);
	strcpy(info[num-1].toFile,destination);
	info[num-1].legth = file_size-(num-1) * offset;
	info[num-1].offset = (num-1) * offset;
	info[num-1].filesize = file_size;
	for(int i = 0; i < num; ++i) //创建number个线程来实现文件的拷贝
	{
		pthread_create(&tid[i],NULL,doThread,&info[i]);
	}
	for(int j = 0; j < num; ++j) //主线程回收创建的number个线程
	{
		pthread_join(tid[j],NULL);
	}
}

void* copy_folder(void * arg)
{
	Info *info = (Info*)arg;
	char destination[255], source[255];
	strcpy(source, info->fromFile);
	strcpy(destination, info->toFile);

	pthread_t *tid;
	tid = (pthread_t *)malloc(sizeof(pthread_t));
	
	int fd_src, thread_num;
	long int file_size;
	if(!opendir(destination))
	{
		if(mkdir(destination, 0777))
		{
			printf("Mkdir Failed %s \n", destination);
		}
	}
	char* path;
	path = malloc(512);
	path = strcat(path, source);

	struct dirent* filename;
	DIR* dp = opendir(path);
	int count = 0;
	while(filename = readdir(dp))
	{
		Info cpfile;
		memset(path, 0, sizeof(path));
		path = strcat(path, source);

		char *file_source, *file_dest;
		file_source = malloc(512);
		file_dest = malloc(512);
		strcat(file_source, source);
		strcat(file_dest, destination);

		if(!endwith(source, '/'))	strcat(file_source, "/");
		if(!endwith(destination, '/'))	strcat(file_dest, "/");

		strcat(file_source, filename->d_name);
		strcat(file_dest, filename->d_name);
		// printf("%d.source is:%s, destination is: %s \n", count++, file_source, file_dest);
		strcpy(cpfile.fromFile, file_source);
		strcpy(cpfile.toFile, file_dest);
		if(is_dir(file_source))
		{
			if(!endwith(file_source, '.'))
				{
					pthread_create(tid, NULL, copy_folder, &cpfile);
					pthread_join(*tid, NULL);
				}
				// copy_folder(file_source, file_dest);
		}
		else
		{
			fd_src = open(file_source, O_RDONLY);
			file_size = lseek(fd_src, 0, SEEK_END);
			// thread_num = file_size/(1*1024*1024);// size over 10 MB, use multithread

			copy_file_multithreads(file_source, file_dest, 1);
		}
	}
}


int main(int argc,char *argv[])
{
	Info* info;
	info = (Info *)malloc(sizeof(Info));
	strcpy(info->fromFile,argv[1]);
	strcpy(info->toFile,argv[2]);

	if(argc<=2){
		printf("Need more args");
		exit(1);
	}
	int number;
	if(argc==4){
		number = atoi(argv[3]);
	}
	else{
		number = 1;
	}
	if(is_dir(argv[1])==1)
	{
		copy_folder(info);
	}
	else
	{
		copy_file_multithreads(argv[1], argv[2], number);
	}


	return 0;
}
