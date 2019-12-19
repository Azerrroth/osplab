#include <time.h>
#include <stdio.h>
#include <dirent.h>
#include <stdlib.h>
#include <string.h>
 
char buff[1024];
int len;
int main(int argc, char const *argv[])
{
	FILE *in,*out;
	char const * src_path = argv[1];	//要被拷贝的文件路径
	char const * des_path = argv[2];	//拷贝的文件放在哪里（路径）
 
	in = fopen(argv[1],"r+");
	out = fopen(argv[2],"w+");
 
	while(len = fread(buff,1,sizeof(buff),in))
	{
		fwrite(buff,1,len,out);
	}
	return 0;
}

