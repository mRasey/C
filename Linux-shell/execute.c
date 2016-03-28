#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <fcntl.h>
#include <math.h>
#include <errno.h>
#include <signal.h>
#include <stddef.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/ioctl.h>
#include <sys/termios.h>
#include <stdio.h>
#include <dirent.h>
#include "global.h"

#define DEBUG
#define STD_INPUT 0
#define STD_OUTPUT 1




int fd[2];
int goon = 0;       //用于设置signal信号量
char *envPath[10], cmdBuff[40];  //外部命令的存放路径及读取外部命令的缓冲空间
History history;                 //历史命令
Job *head = NULL;                //作业头指针
pid_t fgPid;                     //当前前台作业的进程号

/*******************************************************
                  工具以及辅助方法
********************************************************/
/*判断命令是否存在*/
int exists(char *cmdFile){
    int i = 0;
    if((cmdFile[0] == '/' || cmdFile[0] == '.') && access(cmdFile, F_OK) == 0){ //命令在当前目录
        strcpy(cmdBuff, cmdFile);
        return 1;
    }else{  //查找ysh.conf文件中指定的目录，确定命令是否存在
        while(envPath[i] != NULL){ //查找路径已在初始化时设置在envPath[i]中
            strcpy(cmdBuff, envPath[i]);
            strcat(cmdBuff, cmdFile);
            
            if(access(cmdBuff, F_OK) == 0){ //命令文件被找到
                return 1;
            }
            
            i++;
        }
    }
    
    return 0; 
}

/*将字符串转换为整型的Pid*/
int str2Pid(char *str, int start, int end){
    int i, j;
    char chs[20];
    
    for(i = start, j= 0; i < end; i++, j++){
        if(str[i] < '0' || str[i] > '9'){
            return -1;
        }else{
            chs[j] = str[i];
        }
    }
    chs[j] = '\0';
    
    return atoi(chs);
}

/*调整部分外部命令的格式*/
void justArgs(char *str){
    int i, j, len;
    len = strlen(str);
    
    for(i = 0, j = -1; i < len; i++){
        if(str[i] == '/'){
            j = i;
        }
    }

    if(j != -1){ //找到符号'/'
        for(i = 0, j++; j < len; i++, j++){
            str[i] = str[j];
        }
        str[i] = '\0';
    }
}

/*设置goon*/
void setGoon(){
    goon = 1;
}

/*释放环境变量空间*/
void release(){
    int i;
    for(i = 0; strlen(envPath[i]) > 0; i++){
        free(envPath[i]);
    }
}

/*******************************************************
                  信号以及jobs相关
********************************************************/
/*添加新的作业*/
Job* addJob(pid_t pid){
    Job *now = NULL, *last = NULL, *job = (Job*)malloc(sizeof(Job));
    
	//初始化新的job
    job->pid = pid;
    strcpy(job->cmd, inputBuff);
    strcpy(job->state, RUNNING);
    job->next = NULL;
    
    if(head == NULL){ //若是第一个job，则设置为头指针
        head = job;
    }else{ //否则，根据pid将新的job插入到链表的合适位置
		now = head;
		while(now != NULL && now->pid < pid){
			last = now;
			now = now->next;
		}
        last->next = job;
        job->next = now;
    }
    
    return job;
}

/*移除一个作业*/
void rmJob(int pid){
    //pid_t pid;
    
    Job *now = NULL, *last = NULL;
    now = head;
    while(now != NULL && now->pid != pid){
        last = now;
        now = now->next;
    }
    
    if(now == NULL){ //作业不存在，则不进行处理直接返回
        return;
    }
    
    //开始移除该作业
    if(now == head){
        head = now->next;
    }else{
        last->next = now->next;
    }
    
    free(now);
}
//
void ctrl_C(){
    Job *now = NULL;
    
    if(fgPid == 0){ //前台没有作业则直接返回
        return;
    }
    
    now = head;
    while(now != NULL && now->pid != fgPid)
        now = now->next;
    
    if(now != NULL){ //未找到前台作业
        return;
    }    
    
    fgPid = 0;
}



/*组合键命令ctrl+z*/
void ctrl_Z(){    
    Job *now = NULL;;
    if(fgPid == 0){ //前台没有作业则直接返回        
        return;
    }
    
    now = head;
    while(now != NULL && now->pid != fgPid)
        now = now->next;
    
    if(now == NULL){ //未找到前台作业，则根据fgPid添加前台作业
        now = addJob(fgPid);
    }
    
    //修改前台作业的状态及相应的命令格式，并打印提示信息
    strcpy(now->state, STOPPED); 
    now->cmd[strlen(now->cmd)] = '&';
    now->cmd[strlen(now->cmd) + 1] = '\0';
    printf("[%d]\t%s\t\t%s\n", now->pid, now->state, now->cmd);
    
    //发送SIGSTOP信号给正在前台运作的工作，将其停止
    //tcsetpgrp(0,getpid());
    
    kill(fgPid, SIGSTOP);
    fgPid = 0;
}

/*fg命令*/
void fg_exec(int pid){    
    Job *now = NULL; 
    int i;
    
    //根据pid查找作业
    now = head;
    while(now != NULL && now->pid != pid)
        now = now->next;
    
    if(now == NULL){ //未找到作业
        printf("pid为%d 的作业不存在！\n", pid);
        return;
    }

    //记录前台作业的pid，修改对应作业状态
    fgPid = now->pid;
    strcpy(now->state, RUNNING);


    signal(SIGTSTP, ctrl_Z); //设置signal信号，为下一次按下组合键Ctrl+Z做准备
    i = strlen(now->cmd) - 1;
    while(i >= 0 && now->cmd[i] != '&')
        i--;
    now->cmd[i] = '\0';
    
    printf("%s\n", now->cmd);
    tcsetpgrp(0, now->pid);
    kill(now->pid, SIGCONT);//向对象作业发送SIGCONT信号，使其运行
    sleep(1);
    waitpid(fgPid, NULL, WUNTRACED);//父进程等待前台进程的运行
    tcsetpgrp(0,getpid()); 
}

/*bg命令*/
void bg_exec(int pid){
    Job *now = NULL;
    
    //根据pid查找作业
    now = head;
    while(now != NULL && now->pid != pid)
        now = now->next;
    
    if(now == NULL){ //未找到作业
        printf("pid为%d 的作业不存在！\n", pid);
        return;
    }
    
    strcpy(now->state, RUNNING); //修改对象作业的状态
    printf("[%d]\t%s\t\t%s\n", now->pid, now->state, now->cmd);
    
    kill(now->pid, SIGCONT); //向对象作业发送SIGCONT信号，使其运行
}

void mySIGCHLD(int sig, siginfo_t *sip, void* noused){
    if(sip->si_code == CLD_EXITED){//zheng chang jie shu
        rmJob(sip->si_pid);
        fgPid = 0;
    }
    else if(sip->si_code == CLD_KILLED){//ctrl C
        rmJob(sip->si_pid);
        ctrl_C();   
    }
    else if(sip->si_code == CLD_STOPPED){//ctrl Z
        ctrl_Z();
    }
    else if(sip->si_code == CLD_CONTINUED){//fg bg
        
    }
}

/*******************************************************
                    命令历史记录
********************************************************/
void addHistory(char *cmd){
    if(history.end == -1){ //第一次使用history命令
        history.end = 0;
        strcpy(history.cmds[history.end], cmd);
        return;
	}
    
    history.end = (history.end + 1)%HISTORY_LEN; //end前移一位
    strcpy(history.cmds[history.end], cmd); //将命令拷贝到end指向的数组中
    
    if(history.end == history.start){ //end和start指向同一位置
        history.start = (history.start + 1)%HISTORY_LEN; //start前移一位
    }
}

/*******************************************************
                     初始化环境
********************************************************/
/*通过路径文件获取环境路径*/
void getEnvPath(int len, char *buf){
    int i, j, last = 0, pathIndex = 0, temp;
    char path[40];
    
    for(i = 0, j = 0; i < len; i++){
        if(buf[i] == ':'){ //将以冒号(:)分隔的查找路径分别设置到envPath[]中
            if(path[j-1] != '/'){
                path[j++] = '/';
            }
            path[j] = '\0';
            j = 0;
            
            temp = strlen(path);
            envPath[pathIndex] = (char*)malloc(sizeof(char) * (temp + 1));
            strcpy(envPath[pathIndex], path);
            
            pathIndex++;
        }else{
            path[j++] = buf[i];
        }
    }
    
    envPath[pathIndex] = NULL;
}

/*初始化操作*/
void init(){
    int fd, n, len;
    char c, buf[80];

	//打开查找路径文件ysh.conf
    if((fd = open("ysh.conf", O_RDONLY, 660)) == -1){
        perror("init environment failed\n");
        exit(1);
    }
    
	//初始化history链表
    history.end = -1;
    history.start = 0;
    
    len = 0;
	//将路径文件内容依次读入到buf[]中
    while(read(fd, &c, 1) != 0){ 
        buf[len++] = c;
    }
    buf[len] = '\0';

    //将环境路径存入envPath[]
    getEnvPath(len, buf); 
    
    struct sigaction action1;
    action1.sa_handler = mySIGCHLD;
    sigfillset(&action1.sa_mask);
    action1.sa_flags = SA_SIGINFO;
    sigaction(SIGCHLD, &action1, NULL);

    signal(SIGTSTP, SIG_IGN);
    signal(SIGINT,SIG_IGN);
    signal(SIGTTOU, SIG_IGN);

}

/*******************************************************
                      命令执行
********************************************************/
/*执行外部命令*/
void execOuterCmd(SimpleCmd *cmd){
    pid_t pid;
    int pipeIn, pipeOut;
    if(exists(cmd->args[0])){ //命令存在
		signal(SIGUSR1, setGoon);

        if((pid = fork()) < 0){
            perror("fork failed");
            return;
        }

        if(pid == 0){ //子进程
	        signal(SIGTSTP, SIG_DFL);
            signal(SIGINT, SIG_DFL);
            setpgid(0, getpid());

            if(cmd->input != NULL){ //存在输入重定向
                if((pipeIn = open(cmd->input, O_RDONLY, S_IRUSR|S_IWUSR)) == -1){
                    printf("不能打开文件 %s！\n", cmd->input);
                    return;
                }
                if(dup2(pipeIn, 0) == -1){
                    printf("重定向标准输入错误！\n");
                    return;
                }
            }
            
            if(cmd->output != NULL){ //存在输出重定向
                if((pipeOut = open(cmd->output, O_WRONLY|O_CREAT|O_TRUNC, S_IRUSR|S_IWUSR)) == -1){
                    printf("不能打开文件 %s！\n", cmd->output);
                    return ;
                }
                if(dup2(pipeOut, 1) == -1){
                    printf("重定向标准输出错误！\n");
                    return;
                }
            }
            
            if(cmd->isBack){ //若是后台运行命令，等待父进程增加作业
                //signal(SIGUSR1, setGoon); //收到信号，setGoon函数将goon置1，以跳出下面的循环
                while(goon == 0) ; //等待父进程SIGUSR1信号，表示作业已加到链表中
		
                goon = 0; //置0，为下一命令做准备

                //printf("[%d]\t%s\t\t%s\n", getpid(), RUNNING, inputBuff);
                kill(getppid(), SIGUSR1);
            }

            justArgs(cmd->args[0]);
            if(execv(cmdBuff, cmd->args) < 0){ //执行命令
                printf("execv failed!\n");
                return;
            }
        }
		else{ //父进程
            if(cmd ->isBack){ //后台命令             
                fgPid = 0; //pid置0，为下一命令做准备
                addJob(pid); //增加新的作业
                kill(pid, SIGUSR1); //子进程发信号，表示作业已加入
                
                //等待子进程输出
                
                while(goon == 0) ;
                goon = 0;
                printf("[%d]\t%s\t\t%s\n", getpid(), RUNNING, inputBuff);
            }else{ //非后台命令
                fgPid = pid;
                tcsetpgrp(0, pid);
                waitpid(fgPid, NULL, WCONTINUED);//父进程等待前台进程的运行
                tcsetpgrp(0, getpgrp());                
            }
		}
    }else{ //命令不存在

        printf("找不到命令 %s\n", inputBuff);
    }
}

/*执行命令*/
void execSimpleCmd(SimpleCmd *cmd){
    int i, pid;
    char *temp;
    char *regex_out;
    Job *now = NULL;

    if(strcmp(cmd->args[0], "exit") == 0) { //exit命令
        exit(0);
    } else if (strcmp(cmd->args[0], "history") == 0) { //history命令
        if(history.end == -1){
            printf("尚未执行任何命令\n");
            return;
        }
        i = history.start;
        do {
            printf("%s\n", history.cmds[i]);
            i = (i + 1)%HISTORY_LEN;
        } while(i != (history.end + 1)%HISTORY_LEN);
    } else if (strcmp(cmd->args[0], "jobs") == 0) { //jobs命令
        if(head == NULL){
            printf("尚无任何作业\n");
        } else {

            printf("index\tpid\tstate\t\tcommand\n");
            for(i = 1, now = head; now != NULL; now = now->next, i++){
                printf("%d\t%d\t%s\t\t%s\n", i, now->pid, now->state, now->cmd);
            }
        }
    } else if (strcmp(cmd->args[0], "cd") == 0) { //cd命令
        temp = cmd->args[1];
        
        if(temp != NULL){
            if(chdir(temp) < 0){
                if (matchRegex(temp,&regex_out)){
                    chdir(regex_out);
                    free(regex_out);
                }
                else
                    printf("cd; %s 错误的文件名或文件夹名！\n", temp);
            }
        }
    } else if (strcmp(cmd->args[0], "fg") == 0) { //fg命令
        temp = cmd->args[1];
        if(temp != NULL && temp[0] == '%'){
            pid = str2Pid(temp, 1, strlen(temp));
            if(pid != -1){
                fg_exec(pid);
            }
        }else{
            printf("fg; 参数不合法，正确格式为：fg %<int>\n");
        }
    } else if (strcmp(cmd->args[0], "bg") == 0) { //bg命令
        temp = cmd->args[1];
        if(temp != NULL && temp[0] == '%'){
            pid = str2Pid(temp, 1, strlen(temp));
            
            if(pid != -1){
                bg_exec(pid);
            }
        }
		else{
            printf("bg; 参数不合法，正确格式为：bg %<int>\n");
        }
    } else if (strcmp(cmd->args[0], "which") == 0){ //which
        if(whichFunc(cmd->args[1])){
            printf("/bin/%s\n", cmd->args[1]);
        }
        else{
            printf("the cmd is not exist\n");
        }

    } else if(strcmp(cmd->args[0], "echo") == 0){ //which
        for(i=1;cmd->args[i]!=NULL;i++){
            printf("%s ",cmd->args[i]);
        }
        printf("myEcho\n");
    } else{ //外部命令
        execOuterCmd(cmd);
    }
    
    //释放结构体空间
    
}
/*
pipe cmd execute function
*/
void pipeCmd(SimpleCmd *cmd,SimpleCmd *cmd2){
    pid_t pid,i;
    int pipeIn, pipeOut;

    i=fork();
    if(i){
        waitpid(i,NULL,0);
    }
    else{        
        if (exists(cmd->args[0])){
            pipe(fd);
            if((pid = fork()) < 0){
                perror("fork failed");
                return;
            }
            if (pid){
                close(fd[0]);
                close(STD_OUTPUT);
                dup(fd[1]); //把标识符1作为管道的另一个写指针
                close(fd[1]); //关闭管道原有的写指针
                if(cmd->input != NULL){ //存在输入重定向
                    if((pipeIn = open(cmd->input, O_RDONLY, S_IRUSR|S_IWUSR)) == -1){
                        printf("不能打开文件 %s！\n", cmd->input);
                        return;
                    }
                    if(dup2(pipeIn, 0) == -1){
                        printf("重定向标准输入错误！\n");
                        return;
                    }
                }
            
                if(cmd->output != NULL){ //存在输出重定向
                    if((pipeOut = open(cmd->output, O_WRONLY|O_CREAT|O_TRUNC, S_IRUSR|S_IWUSR)) == -1){
                        printf("不能打开文件 %s！\n", cmd->output);
                        return ;
                    }
                    if(dup2(pipeOut, 1) == -1){
                        printf("重定向标准输出错误！\n");
                        return;
                    }
                }

                justArgs(cmd->args[0]);
                if(execv(cmdBuff, cmd->args) < 0){ //执行命令
                    printf("execv failed!\n");
                    return;
                }               //用新文件覆盖当前程序
                printf("-----father failed.\n");            

            }
            else{
                if (exists(cmd2->args[0])){
                    close(fd[1]);
                    close(STD_INPUT);
                    dup(fd[0]); //把标识符0作为管道的另一个读指针
                    close(fd[0]); //关闭管道原有的读指针
                    if(cmd2->input != NULL){ //存在输入重定向
                        if((pipeIn = open(cmd2->input, O_RDONLY, S_IRUSR|S_IWUSR)) == -1){
                            printf("不能打开文件 %s！\n", cmd2->input);
                            return;
                        }
                        if(dup2(pipeIn, 0) == -1){
                            printf("重定向标准输入错误！\n");
                            return;
                        }
                    }
                
                    if(cmd2->output != NULL){ //存在输出重定向
                        if((pipeOut = open(cmd2->output, O_WRONLY|O_CREAT|O_TRUNC, S_IRUSR|S_IWUSR)) == -1){
                            printf("不能打开文件 %s！\n", cmd2->output);
                            return ;
                        }
                        if(dup2(pipeOut, 1) == -1){
                            printf("重定向标准输出错误！\n");
                            return;
                        }
                    }

                    justArgs(cmd2->args[0]);
                    if(execv(cmdBuff, cmd2->args) < 0){ //执行命令
                        printf("execv failed!\n");
                        return;
                    }               //用新文件覆盖当前程序
                    printf("-----child failed.\n");
                }
            }
        }else{ //命令不存在
            printf("找不到命令 15%s\n", inputBuff);
        }
     }   

}
/*
regex match function
*/
int matchRegex(char *pattern,char **filename){
    DIR *dp;
    struct dirent *entry;
    struct stat statbuf;
    char buf[100];
    int i,j,last_index_file,last_index;

    getcwd(buf, sizeof(buf));

    if((dp = opendir(&buf)) == NULL) {
        fprintf(stderr,"cannot open directory: %s\n", buf);
        return;
    }
    while((entry = readdir(dp)) != NULL) {
        lstat(entry->d_name,&statbuf);
        i=0;
        j=0;
        last_index=strlen(pattern)-1;
        last_index_file=strlen(entry->d_name)-1;
        while (i<=last_index_file&&j<=last_index){
            if (entry->d_name[i]!=pattern[j])
                break;
            i++;
            j++;
        }
        if (i==strlen(entry->d_name)||pattern[j]!='*'){
            continue;
        }
        else {
            while (last_index>=j&&last_index_file>=i){
                if (pattern[last_index]!=entry->d_name[last_index_file])
                    break;
                last_index--;
                last_index_file--;
            }
            if (last_index==j){
                *filename = (char*)malloc(sizeof(char) * (strlen(entry->d_name) + 1));
                strcpy(*filename,entry->d_name);
                return 1;
            }
        }
    }
    closedir(dp);
    return 0;
}
/*
which function
*/
int whichFunc(char *cmd_name){
    DIR *dp;
    struct dirent *entry;
    struct stat statbuf;

    if((dp = opendir("/bin")) == NULL) {
        fprintf(stderr,"cannot open directory: %s\n", "/bin");
        return;
    }
    while((entry = readdir(dp)) != NULL) {
        if(strcmp(entry->d_name,cmd_name)==0)
            return 1;
    }
    closedir(dp);
    return 0;    
}
