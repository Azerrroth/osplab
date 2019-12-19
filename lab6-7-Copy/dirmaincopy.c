#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>

char buff[1024];
int len;
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
	fclose(out);
	fclose(in);
	return 0;
}

