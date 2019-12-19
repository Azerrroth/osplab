#define GET_ARRAY_LEN(array, len) { len = sizeof(array)/sizeof(array[0]);}
#define LENGTH 1024
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pwd.h>
#include <string.h>
#include <pthread.h>

// Return the current file name to cwd, if the length is longer than len, return NULL.
char* getCurrentPath(char* cwd, size_t len);
// Return the path for current user to login in. 
int getuserdir (char *aoUserDir);

// Parse the cmd and trans to do_exe
void parse(char* buf);
// Execute the cmd
void do_exe(char* buf, char* argv[]);
// Execute the outer cmd
void execOutCmd (char* buf, char* argv[]);

char usrDir[LENGTH];

int main()
{
    char cmd[LENGTH], input[LENGTH];
    char hostname[LENGTH];
    char login[LENGTH];
    char cwd[LENGTH], cpwd[LENGTH]; // cwd is the current Folder name, cpwd is the full path of the 
    int i, index;
    sprintf(login, getlogin());
    gethostname(hostname, LENGTH);
    getuserdir(usrDir);
    // printf("%s\n", getenv("PIP"));
    // \033[0m 是控制码  40 是背景颜色黑色，可以去掉 32绿色 34 蓝色 36 蓝绿色
    while(1)
    {
        getcwd(cpwd, LENGTH);
        getCurrentPath(cwd, LENGTH);
        if (strcmp(cpwd, usrDir) == 0) 
        {
            printf("\033[40;32m[%s@%s ~]$\033[0m ", login, hostname); 
        }
        else if (strcmp(cpwd, "/") == 0) {
            printf("\033[40;32m[%s@%s /]$\033[0m ", login, hostname); 
        }
        else 
        {
            printf("\033[40;32m[%s@%s %s]$\033[0m ", login, hostname, cwd); 
        }

        memset(cmd, 0x00, sizeof(cmd));
        while(scanf("%[^\n]%*c", cmd) == 0)
        { 
            if (strcmp(cpwd, usrDir) == 0) 
            {
                printf("\033[40;32m[%s@%s ~]$\033[0m ", login, hostname); 
            }
            else if (strcmp(cpwd, "/") == 0) {
                printf("\033[40;32m[%s@%s /]$\033[0m ", login, hostname); 
            }
            else 
            {
                printf("\033[40;32m[%s@%s %s]$\033[0m ", login, hostname, cwd); 
            }
            while(getchar() != '\n');
        }
        parse(cmd);
    }
}

char* getCurrentPath(char* cwd, size_t len) 
{
    char ans[1024];
    int i, index, length;
    GET_ARRAY_LEN(ans, length);
    getcwd(ans, length);
    for (i = 0; i < 80; i++) {
        if (ans[i] == '/') {
            index = i;
        }
        if (ans[i] == 0) {
            break;
        }
    }
    index++;
    if (len < i-index) {
        printf("ERROR: %d < %d\n", len, i-index);
        return NULL;
    }
    strcpy(cwd, ans + index);
    return cwd;
}

int getuserdir (char *aoUserDir) 
{
    char *LoginId;
    struct passwd *pwdinfo;
    if (aoUserDir == NULL)  return -9;
    if ((LoginId = getlogin ()) == NULL)
    {
        perror ("getlogin");
        aoUserDir[0] = '\0';
        return -8;
    }
    if ((pwdinfo = getpwnam (LoginId)) == NULL)
    {
        perror ("getpwnam");
        return -7;
    }
    strcpy (aoUserDir, pwdinfo->pw_dir);
}

void do_exe(char* buf, char* argv[]) //加载程序
{
    if (strcmp(buf, "exit") == 0) 
    {
        exit(0);
    } 
    else if (strcmp(buf, "cd") == 0) 
    {
        if (argv[1] == NULL) {
            chdir(usrDir);
        }
        else if (chdir(argv[1]) < 0 ) {
            printf("minish: cd: %s: path not found...\n", argv[1]);
        }
    }
    else 
    {
        execOutCmd(buf, argv);
    }
}

void execOutCmd (char* buf, char* argv[])
{
    pid_t pid = fork();
 
    if(pid == 0)
    {
        execvp(buf, argv);
        perror("fork"); // If command not found, print the error message.
        exit(1);
    }
    wait(NULL); // wait until the process end. Recycle the memory.
}

void parse(char* buf)
{
    char* argv[8] = {};
    int argc = 0;
    int status = 0;
    int i, anotherCmd = 0;
    // printf("The Command is: %s \n", buf);
    for(i = 0; buf[i] != 0; ++i){
        if(status == 0 && !isspace(buf[i])){
            argv[argc++] = buf + i;
            status = 1;
        }
        else if(isspace(buf[i])){
            status = 0;
            buf[i] = 0;
        }
        else if(buf[i] == ';') {
            buf[i] = 0;
            anotherCmd = 1;
            break;
        }
    }
    argv[argc] = NULL;
    do_exe(buf, argv);
    if (anotherCmd == 1)
        parse(buf + i + 1);

}