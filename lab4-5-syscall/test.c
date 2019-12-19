#include <unistd.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#define __NR_schello 436
int execmd(char* cmd,char* result) 
{

	char buffer[128];                         //定义缓冲区                        
    	FILE* pipe = popen(cmd, "r");            //打开管道，并执行命令 
    	if (!pipe)
         	 return 0;                      //返回0表示运行失败 

 	while(!feof(pipe)) 
	{
		if(fgets(buffer, 128, pipe))
		{             //将管道输出到result
			strcat(result,buffer);
		}
	}
	pclose(pipe);                            //关闭管道 
	return 1;                                 //返回1表示运行成功 
}
int main(int argc, char *argv[])
{
	syscall(__NR_schello);
	printf("ok! run dmesg | grep hello in terminal\n");
	printf("%d", sizeof(char));
	char result[1024*256] = "";
	char process[1024*256] = "";
	char time[5] = "";
	char c = '[';
	char cmd[30] = "dmesg | grep ";
	if(execmd("dmesg | grep Hello", result) == 1)
	{
		strncpy(time, strrchr(result, c) + 2, 4);
		printf(strrchr(result,c));
		printf("%s\n",strcat(cmd, time));
		if(execmd(cmd, process) == 1)
			printf(process);
	}
	return 0;
}
