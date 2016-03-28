#ifndef _global_H
#define _global_H

#ifdef	__cplusplus
extern "C" {
#endif   
    
    #define HISTORY_LEN 10
    
    #define STOPPED "stopped"
    #define RUNNING "running"
    #define DONE    "done"
    #define KNRM "\x1B[0m"   
    #define KRED "\x1B[31m"
    #define KGRN "\x1B[34;32m\033[1m"
    #define KYEL "\x1B[33m"
    #define KBLU "\x1B[34m"
    #define KMAG "\x1B[35m"
    #define KCYN "\x1B[36m"
    #define KWHT "\x1B[37m"
    #define CLER "\033[2J"
    #include <stdio.h>
    #include <stdlib.h>
	#include <string.h>
    
    typedef struct SimpleCmd {
        int isBack;     // �Ƿ��̨����
        char **args;    // �������
        char *input;    // �����ض���
        char *output;   // ����ض���
    } SimpleCmd;

    typedef struct History {
        int start;                    //��λ��
        int end;                      //ĩλ��
        char cmds[HISTORY_LEN][100];  //��ʷ����
    } History;

    typedef struct Job {
        int pid;          //���̺�
        char cmd[100];    //������
        char state[10];   //��ҵ״̬
        struct Job *next; //��һ�ڵ�ָ��
    } Job;
    
    char inputBuff[100];  //������������
    int isPipe;
    void init();
    void addHistory(char *history);
    void execute();

#ifdef	__cplusplus
}
#endif

#endif	/* _global_H */
