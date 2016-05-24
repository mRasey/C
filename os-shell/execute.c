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
int goon = 0;       //��������signal�ź���
char *envPath[10], cmdBuff[40];  //�ⲿ����Ĵ��·������ȡ�ⲿ����Ļ���ռ�
History history;                 //��ʷ����
Job *head = NULL;                //��ҵͷָ��
pid_t fgPid;                     //��ǰǰ̨��ҵ�Ľ��̺�

/*******************************************************
                  �����Լ���������
********************************************************/
/*�ж������Ƿ����*/
int exists(char *cmdFile){
    int i = 0;
    if((cmdFile[0] == '/' || cmdFile[0] == '.') && access(cmdFile, F_OK) == 0){ //�����ڵ�ǰĿ¼
        strcpy(cmdBuff, cmdFile);
        return 1;
    }else{  //����ysh.conf�ļ���ָ����Ŀ¼��ȷ�������Ƿ����
        while(envPath[i] != NULL){ //����·�����ڳ�ʼ��ʱ������envPath[i]��
            strcpy(cmdBuff, envPath[i]);
            strcat(cmdBuff, cmdFile);
            
            if(access(cmdBuff, F_OK) == 0){ //�����ļ����ҵ�
                return 1;
            }
            
            i++;
        }
    }
    
    return 0; 
}

/*���ַ���ת��Ϊ���͵�Pid*/
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

/*���������ⲿ����ĸ�ʽ*/
void justArgs(char *str){
    int i, j, len;
    len = strlen(str);
    
    for(i = 0, j = -1; i < len; i++){
        if(str[i] == '/'){
            j = i;
        }
    }

    if(j != -1){ //�ҵ�����'/'
        for(i = 0, j++; j < len; i++, j++){
            str[i] = str[j];
        }
        str[i] = '\0';
    }
}

/*����goon*/
void setGoon(){
    goon = 1;
}

/*�ͷŻ��������ռ�*/
void release(){
    int i;
    for(i = 0; strlen(envPath[i]) > 0; i++){
        free(envPath[i]);
    }
}

/*******************************************************
                  �ź��Լ�jobs���
********************************************************/
/*����µ���ҵ*/
Job* addJob(pid_t pid){
    Job *now = NULL, *last = NULL, *job = (Job*)malloc(sizeof(Job));
    
	//��ʼ���µ�job
    job->pid = pid;
    strcpy(job->cmd, inputBuff);
    strcpy(job->state, RUNNING);
    job->next = NULL;
    
    if(head == NULL){ //���ǵ�һ��job��������Ϊͷָ��
        head = job;
    }else{ //���򣬸���pid���µ�job���뵽����ĺ���λ��
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

/*�Ƴ�һ����ҵ*/
void rmJob(int pid){
    //pid_t pid;
    
    Job *now = NULL, *last = NULL;
    now = head;
    while(now != NULL && now->pid != pid){
        last = now;
        now = now->next;
    }
    
    if(now == NULL){ //��ҵ�����ڣ��򲻽��д���ֱ�ӷ���
        return;
    }
    
    //��ʼ�Ƴ�����ҵ
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
    
    if(fgPid == 0){ //ǰ̨û����ҵ��ֱ�ӷ���
        return;
    }
    
    now = head;
    while(now != NULL && now->pid != fgPid)
        now = now->next;
    
    if(now != NULL){ //δ�ҵ�ǰ̨��ҵ
        return;
    }    
    
    fgPid = 0;
}



/*��ϼ�����ctrl+z*/
void ctrl_Z(){    
    Job *now = NULL;;
    if(fgPid == 0){ //ǰ̨û����ҵ��ֱ�ӷ���        
        return;
    }
    
    now = head;
    while(now != NULL && now->pid != fgPid)
        now = now->next;
    
    if(now == NULL){ //δ�ҵ�ǰ̨��ҵ�������fgPid���ǰ̨��ҵ
        now = addJob(fgPid);
    }
    
    //�޸�ǰ̨��ҵ��״̬����Ӧ�������ʽ������ӡ��ʾ��Ϣ
    strcpy(now->state, STOPPED); 
    now->cmd[strlen(now->cmd)] = '&';
    now->cmd[strlen(now->cmd) + 1] = '\0';
    printf("[%d]\t%s\t\t%s\n", now->pid, now->state, now->cmd);
    
    //����SIGSTOP�źŸ�����ǰ̨�����Ĺ���������ֹͣ
    //tcsetpgrp(0,getpid());
    
    kill(fgPid, SIGSTOP);
    fgPid = 0;
}

/*fg����*/
void fg_exec(int pid){    
    Job *now = NULL; 
    int i;
    
    //����pid������ҵ
    now = head;
    while(now != NULL && now->pid != pid)
        now = now->next;
    
    if(now == NULL){ //δ�ҵ���ҵ
        printf("pidΪ%d ����ҵ�����ڣ�\n", pid);
        return;
    }

    //��¼ǰ̨��ҵ��pid���޸Ķ�Ӧ��ҵ״̬
    fgPid = now->pid;
    strcpy(now->state, RUNNING);


    signal(SIGTSTP, ctrl_Z); //����signal�źţ�Ϊ��һ�ΰ�����ϼ�Ctrl+Z��׼��
    i = strlen(now->cmd) - 1;
    while(i >= 0 && now->cmd[i] != '&')
        i--;
    now->cmd[i] = '\0';
    
    printf("%s\n", now->cmd);
    tcsetpgrp(0, now->pid);
    kill(now->pid, SIGCONT);//�������ҵ����SIGCONT�źţ�ʹ������
    sleep(1);
    waitpid(fgPid, NULL, WUNTRACED);//�����̵ȴ�ǰ̨���̵�����
    tcsetpgrp(0,getpid()); 
}

/*bg����*/
void bg_exec(int pid){
    Job *now = NULL;
    
    //����pid������ҵ
    now = head;
    while(now != NULL && now->pid != pid)
        now = now->next;
    
    if(now == NULL){ //δ�ҵ���ҵ
        printf("pidΪ%d ����ҵ�����ڣ�\n", pid);
        return;
    }
    
    strcpy(now->state, RUNNING); //�޸Ķ�����ҵ��״̬
    printf("[%d]\t%s\t\t%s\n", now->pid, now->state, now->cmd);
    
    kill(now->pid, SIGCONT); //�������ҵ����SIGCONT�źţ�ʹ������
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
                    ������ʷ��¼
********************************************************/
void addHistory(char *cmd){
    if(history.end == -1){ //��һ��ʹ��history����
        history.end = 0;
        strcpy(history.cmds[history.end], cmd);
        return;
	}
    
    history.end = (history.end + 1)%HISTORY_LEN; //endǰ��һλ
    strcpy(history.cmds[history.end], cmd); //���������endָ���������
    
    if(history.end == history.start){ //end��startָ��ͬһλ��
        history.start = (history.start + 1)%HISTORY_LEN; //startǰ��һλ
    }
}

/*******************************************************
                     ��ʼ������
********************************************************/
/*ͨ��·���ļ���ȡ����·��*/
void getEnvPath(int len, char *buf){
    int i, j, last = 0, pathIndex = 0, temp;
    char path[40];
    
    for(i = 0, j = 0; i < len; i++){
        if(buf[i] == ':'){ //����ð��(:)�ָ��Ĳ���·���ֱ����õ�envPath[]��
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

/*��ʼ������*/
void init(){
    int fd, n, len;
    char c, buf[80];

	//�򿪲���·���ļ�ysh.conf
    if((fd = open("ysh.conf", O_RDONLY, 660)) == -1){
        perror("init environment failed\n");
        exit(1);
    }
    
	//��ʼ��history����
    history.end = -1;
    history.start = 0;
    
    len = 0;
	//��·���ļ��������ζ��뵽buf[]��
    while(read(fd, &c, 1) != 0){ 
        buf[len++] = c;
    }
    buf[len] = '\0';

    //������·������envPath[]
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
                      ����ִ��
********************************************************/
/*ִ���ⲿ����*/
void execOuterCmd(SimpleCmd *cmd){
    pid_t pid;
    int pipeIn, pipeOut;
    if(exists(cmd->args[0])){ //�������
		signal(SIGUSR1, setGoon);

        if((pid = fork()) < 0){
            perror("fork failed");
            return;
        }

        if(pid == 0){ //�ӽ���
	        signal(SIGTSTP, SIG_DFL);
            signal(SIGINT, SIG_DFL);
            setpgid(0, getpid());

            if(cmd->input != NULL){ //���������ض���
                if((pipeIn = open(cmd->input, O_RDONLY, S_IRUSR|S_IWUSR)) == -1){
                    printf("���ܴ��ļ� %s��\n", cmd->input);
                    return;
                }
                if(dup2(pipeIn, 0) == -1){
                    printf("�ض����׼�������\n");
                    return;
                }
            }
            
            if(cmd->output != NULL){ //��������ض���
                if((pipeOut = open(cmd->output, O_WRONLY|O_CREAT|O_TRUNC, S_IRUSR|S_IWUSR)) == -1){
                    printf("���ܴ��ļ� %s��\n", cmd->output);
                    return ;
                }
                if(dup2(pipeOut, 1) == -1){
                    printf("�ض����׼�������\n");
                    return;
                }
            }
            
            if(cmd->isBack){ //���Ǻ�̨��������ȴ�������������ҵ
                //signal(SIGUSR1, setGoon); //�յ��źţ�setGoon������goon��1�������������ѭ��
                while(goon == 0) ; //�ȴ�������SIGUSR1�źţ���ʾ��ҵ�Ѽӵ�������
		
                goon = 0; //��0��Ϊ��һ������׼��

                //printf("[%d]\t%s\t\t%s\n", getpid(), RUNNING, inputBuff);
                kill(getppid(), SIGUSR1);
            }

            justArgs(cmd->args[0]);
            if(execv(cmdBuff, cmd->args) < 0){ //ִ������
                printf("execv failed!\n");
                return;
            }
        }
		else{ //������
            if(cmd ->isBack){ //��̨����             
                fgPid = 0; //pid��0��Ϊ��һ������׼��
                addJob(pid); //�����µ���ҵ
                kill(pid, SIGUSR1); //�ӽ��̷��źţ���ʾ��ҵ�Ѽ���
                
                //�ȴ��ӽ������
                
                while(goon == 0) ;
                goon = 0;
                printf("[%d]\t%s\t\t%s\n", getpid(), RUNNING, inputBuff);
            }else{ //�Ǻ�̨����
                fgPid = pid;
                tcsetpgrp(0, pid);
                waitpid(fgPid, NULL, WCONTINUED);//�����̵ȴ�ǰ̨���̵�����
                tcsetpgrp(0, getpgrp());                
            }
		}
    }else{ //�������

        printf("�Ҳ������� %s\n", inputBuff);
    }
}

/*ִ������*/
void execSimpleCmd(SimpleCmd *cmd){
    int i, pid;
    char *temp;
    char *regex_out;
    Job *now = NULL;

    if(strcmp(cmd->args[0], "exit") == 0) { //exit����
        exit(0);
    } else if (strcmp(cmd->args[0], "history") == 0) { //history����
        if(history.end == -1){
            printf("��δִ���κ�����\n");
            return;
        }
        i = history.start;
        do {
            printf("%s\n", history.cmds[i]);
            i = (i + 1)%HISTORY_LEN;
        } while(i != (history.end + 1)%HISTORY_LEN);
    } else if (strcmp(cmd->args[0], "jobs") == 0) { //jobs����
        if(head == NULL){
            printf("�����κ���ҵ\n");
        } else {

            printf("index\tpid\tstate\t\tcommand\n");
            for(i = 1, now = head; now != NULL; now = now->next, i++){
                printf("%d\t%d\t%s\t\t%s\n", i, now->pid, now->state, now->cmd);
            }
        }
    } else if (strcmp(cmd->args[0], "cd") == 0) { //cd����
        temp = cmd->args[1];
        
        if(temp != NULL){
            if(chdir(temp) < 0){
                if (matchRegex(temp,&regex_out)){
                    chdir(regex_out);
                    free(regex_out);
                }
                else
                    printf("cd; %s ������ļ������ļ�������\n", temp);
            }
        }
    } else if (strcmp(cmd->args[0], "fg") == 0) { //fg����
        temp = cmd->args[1];
        if(temp != NULL && temp[0] == '%'){
            pid = str2Pid(temp, 1, strlen(temp));
            if(pid != -1){
                fg_exec(pid);
            }
        }else{
            printf("fg; �������Ϸ�����ȷ��ʽΪ��fg %<int>\n");
        }
    } else if (strcmp(cmd->args[0], "bg") == 0) { //bg����
        temp = cmd->args[1];
        if(temp != NULL && temp[0] == '%'){
            pid = str2Pid(temp, 1, strlen(temp));
            
            if(pid != -1){
                bg_exec(pid);
            }
        }
		else{
            printf("bg; �������Ϸ�����ȷ��ʽΪ��bg %<int>\n");
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
    } else{ //�ⲿ����
        execOuterCmd(cmd);
    }
    
    //�ͷŽṹ��ռ�
    
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
                dup(fd[1]); //�ѱ�ʶ��1��Ϊ�ܵ�����һ��дָ��
                close(fd[1]); //�رչܵ�ԭ�е�дָ��
                if(cmd->input != NULL){ //���������ض���
                    if((pipeIn = open(cmd->input, O_RDONLY, S_IRUSR|S_IWUSR)) == -1){
                        printf("���ܴ��ļ� %s��\n", cmd->input);
                        return;
                    }
                    if(dup2(pipeIn, 0) == -1){
                        printf("�ض����׼�������\n");
                        return;
                    }
                }
            
                if(cmd->output != NULL){ //��������ض���
                    if((pipeOut = open(cmd->output, O_WRONLY|O_CREAT|O_TRUNC, S_IRUSR|S_IWUSR)) == -1){
                        printf("���ܴ��ļ� %s��\n", cmd->output);
                        return ;
                    }
                    if(dup2(pipeOut, 1) == -1){
                        printf("�ض����׼�������\n");
                        return;
                    }
                }

                justArgs(cmd->args[0]);
                if(execv(cmdBuff, cmd->args) < 0){ //ִ������
                    printf("execv failed!\n");
                    return;
                }               //�����ļ����ǵ�ǰ����
                printf("-----father failed.\n");            

            }
            else{
                if (exists(cmd2->args[0])){
                    close(fd[1]);
                    close(STD_INPUT);
                    dup(fd[0]); //�ѱ�ʶ��0��Ϊ�ܵ�����һ����ָ��
                    close(fd[0]); //�رչܵ�ԭ�еĶ�ָ��
                    if(cmd2->input != NULL){ //���������ض���
                        if((pipeIn = open(cmd2->input, O_RDONLY, S_IRUSR|S_IWUSR)) == -1){
                            printf("���ܴ��ļ� %s��\n", cmd2->input);
                            return;
                        }
                        if(dup2(pipeIn, 0) == -1){
                            printf("�ض����׼�������\n");
                            return;
                        }
                    }
                
                    if(cmd2->output != NULL){ //��������ض���
                        if((pipeOut = open(cmd2->output, O_WRONLY|O_CREAT|O_TRUNC, S_IRUSR|S_IWUSR)) == -1){
                            printf("���ܴ��ļ� %s��\n", cmd2->output);
                            return ;
                        }
                        if(dup2(pipeOut, 1) == -1){
                            printf("�ض����׼�������\n");
                            return;
                        }
                    }

                    justArgs(cmd2->args[0]);
                    if(execv(cmdBuff, cmd2->args) < 0){ //ִ������
                        printf("execv failed!\n");
                        return;
                    }               //�����ļ����ǵ�ǰ����
                    printf("-----child failed.\n");
                }
            }
        }else{ //�������
            printf("�Ҳ������� 15%s\n", inputBuff);
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
