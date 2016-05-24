#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <string.h>
#include <signal.h>
#include <fcntl.h>
#include <time.h>
#include "job.h"

#define CLOCK_PER_SEC 2
#undef DEBUG
#define MYDEBUG
#undef MYDEBUG


int timeCount = 0;
int jobid=0;
int siginfo=1;
int fifo;
int pipe_fd;
int globalfd;

struct waitqueue *head[3]={NULL};
struct waitqueue *next=NULL,*current =NULL;

#ifdef DEBUG
void print_jobInfoInQueue(){
	struct waitqueue *p;
	int i;
	printf("JOBID\tPID\tOWNER\tRUNTIME\tWAITTIME\tSTATE\tDEFPRI\tCURPRI\n");
	if(current){
		printf("%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d  current\n",
			current->job->jid,
			current->job->pid,
			current->job->ownerid,
			current->job->run_time,
			current->job->wait_time,
			current->job->state,
			current->job->defpri,
			current->job->curpri);
	}
	for(i=0;i<3;i++){
		for(p=head[i];p!=NULL;p=p->next){
			printf("%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\n",
				p->job->jid,
				p->job->pid,
				p->job->ownerid,
				p->job->run_time,
				p->job->wait_time,
				p->job->state,
				p->job->defpri,
				p->job->curpri);
		}
	}
    		
}
void debug_print_jobInfoInQueue(){
	struct waitqueue *p;
	char timebuf[BUFLEN];
	int i;

	for(i=0;i<3;i++){
		for(p=head[i];p!=NULL;p=p->next){
			strcpy(timebuf,ctime(&(p->job->create_time)));
			timebuf[strlen(timebuf)-1]='\0';

			printf("%d\t%d\t%d\t%d\t%d\t%s\t%d\t%d\t%d\n",
				p->job->jid,
				p->job->pid,
				p->job->ownerid,
				p->job->run_time,
				p->job->wait_time,
				timebuf,
				p->job->state,
				p->job->defpri,
				p->job->curpri);
		}
	}

}
#endif 
void print_jobInfo(struct waitqueue *p){
	if (p){
		char timebuf[BUFLEN];
		strcpy(timebuf,ctime(&(p->job->create_time)));
		timebuf[strlen(timebuf)-1]='\0';
		printf("%d\t%d\t%d\t%d\t%d\t%s\t%d\t%d\t%d\n",
			p->job->jid,
			p->job->pid,
			p->job->ownerid,
			p->job->run_time,
			p->job->wait_time,
			timebuf,
			p->job->state,
			p->job->defpri,
			p->job->curpri);
	}
}

void Free (struct waitqueue *p){
	int i;
	for(i = 0;(p->job->cmdarg)[i] != NULL; i++){
			free((p->job->cmdarg)[i]);
			(p->job->cmdarg)[i] = NULL;
		}
		/* ÊÍ·Å¿Õ¼ä */
		free(p->job->cmdarg);
		free(p->job);
		free(p);
}
void setTimer(int pri){

	if(pri == 1){
		timeCount=5*CLOCK_PER_SEC;
	}else if(pri == 2){
		timeCount=2*CLOCK_PER_SEC;
	}else if(pri == 3){
		timeCount=1*CLOCK_PER_SEC;
	}else{
		/*error */
		printf("priority error:settime\n");
	}
}

void changeQueue(struct waitqueue *p,int prevPri,int nowPri){
	int prevNum = 3-prevPri;
	int nowNum = 3-nowPri;
	struct waitqueue *prev,*q;

	if(head[prevNum] == p){
		head[prevNum]=p->next;
	}
	else{
		for(prev = head[prevNum],q = prev->next;q!=p;prev=q,q=q->next);
		prev->next = q->next;
	}
	p->next = NULL;

	if(!head[nowNum]){
		head[nowNum]=p;
	}
	else{
		for(q = head[nowNum];q->next!=NULL;q=q->next);
		q->next=p;
	}

}
/* µ÷¶È³ÌÐò */
void scheduler()
{	
	if(current || (head[0]||head[1]||head[2])){
		#ifdef DEBUG
		printf("scheduler\n");
		print_jobInfoInQueue();
		updateall();
		printf("updateall\n");
		print_jobInfoInQueue();
		/* Ñ¡Ôñ¸ßÓÅÏÈ¼¶×÷Òµ */
		next=jobselect();
		printf("jobselect\n");
		printf("JOBID\tPID\tOWNER\tRUNTIME\tWAITTIME\tCREATTIME\tSTATE\tDEFPRI\tCURPRI\n");
		print_jobInfo(next);
		/* ×÷ÒµÇÐ»» */
		printf("jobswitch_before\n");
		print_jobInfoInQueue();
		jobswitch();
		printf("jobswitch\n");
		print_jobInfoInQueue();
		#else
		updateall();
		next=jobselect();
		jobswitch();
		#endif
	}
}

int allocjid()
{
	return ++jobid;
}
//CHANGE
int getQueueNum(struct jobinfo *job){
	return (3 - job->curpri);
}

void updateall()
{
	struct waitqueue *p;
	int i;
	int pri;
	int currentTimeSec=0;

	if(current){
		pri = current->job->curpri;
		if(pri == 1){
			currentTimeSec=5*CLOCK_PER_SEC;
		}else if(pri == 2){
			currentTimeSec=2*CLOCK_PER_SEC;
		}else if(pri == 3){
			currentTimeSec=1*CLOCK_PER_SEC;
		}
	}

	/* ¸refresh current CHANGE*/
	if(current){
		current->job->run_time += 1; /* ¼Ó1´ú±í1000ms */		
		current->job->curpri = current->job->defpri;
		current->job->wait_time = 0;
	}
	/*refresh waitQueue   CHANGE*/
	for(i=0;i<3;i++){
		for(p = head[i]; p != NULL; p = p->next){
			p->job->wait_time += (currentTimeSec-timeCount*CLOCK_PER_SEC);
			if(p->job->wait_time >= 10*CLOCK_PER_SEC && p->job->curpri < 3){
				changeQueue(p,p->job->curpri,p->job->curpri+1);
				p->job->curpri++;
				p->job->wait_time = 0;
			}
		}
	}
}

struct waitqueue* jobselect()
{
	struct waitqueue *select;
	int highest = -1;
	int i;
	int currentPriNum;

	select = NULL;
	if(current){
		currentPriNum = getQueueNum(current->job);
	}
	else{
		currentPriNum = 2;
	}
	for(i=0;i<=currentPriNum;i++){
		if(head[i]){
			/*select the highest pri   CHANGE*/
			select = head[i];
			head[i] = select->next;	
			break;
		}
	}
	if(select){
		select->next = NULL;
	}
	
	return select;
}

void jobswitch()
{
	struct waitqueue *p;
	int i;
	int queueNum;
	int ret=0,status;

	if(current){
		ret = waitpid(current->job->pid,&status,WNOHANG);
		if(ret){ /* µ±Ç°×÷ÒµÍê³É */
			/* ×÷ÒµÍê³É£¬É¾³ýËü */
			Free(current);
			timeCount =0;
			current = NULL;
		}
	}

	if(next == NULL && current == NULL) /* Ã»ÓÐ×÷ÒµÒªÔËÐÐ */

		return;
	else if (next != NULL && current == NULL){ /* ¿ªÊ¼ÐÂµÄ×÷Òµ */

		printf("begin start new job\n");
		current = next;
		next = NULL;
		current->job->state = RUNNING;
		kill(current->job->pid,SIGCONT);
		setTimer(current->job->curpri);
		return;
	}
	else if (next != NULL && current != NULL){ /* ÇÐ»»×÷Òµ */
		printf("switch to Pid: %d\n",next->job->pid);
		kill(current->job->pid,SIGSTOP);
		current->job->state = READY;

		/* put current back to queue     CHANGE */
		queueNum = getQueueNum(current->job);
		if(head[queueNum]){
			for(p = head[queueNum]; p->next != NULL; p = p->next);
			p->next = current;
		}else{
			head[queueNum] = current;
		}
		current = next;
		next = NULL;
		current->job->state = RUNNING;
		current->job->wait_time = 0;
		kill(current->job->pid,SIGCONT);
		setTimer(current->job->curpri);
		return;
	}else{ /* next == NULLÇÒcurrent != NULL£¬²»ÇÐ»» */
		// kill(current->job->pid,SIGCONT);
		 setTimer(current->job->curpri);
		return;
	}
	
}

void do_enq(struct jobinfo **job,struct jobcmd enqcmd)
{	
	struct jobinfo *newjob;
	struct waitqueue *newnode,*p;
	int i=0,pid;
	char *offset,*argvec,*q;
	char **arglist;
	sigset_t zeromask;
	int queueNum;

	sigemptyset(&zeromask);

	/* ·â×°jobinfoÊý¾Ý½á¹¹ */
	newjob = (struct jobinfo *)malloc(sizeof(struct jobinfo));
	newjob->jid = allocjid();
	newjob->defpri = enqcmd.defpri;
	newjob->curpri = enqcmd.defpri;
	newjob->ownerid = enqcmd.owner;
	newjob->state = READY;
	newjob->create_time = time(NULL);
	newjob->wait_time = 0;
	newjob->run_time = 0;
	arglist = (char**)malloc(sizeof(char*)*(enqcmd.argnum+1));
	newjob->cmdarg = arglist;
	offset = enqcmd.data;
	argvec = enqcmd.data;
	while (i < enqcmd.argnum){
		if(*offset == ':'){
			*offset++ = '\0';
			q = (char*)malloc(offset - argvec);
			strcpy(q,argvec);
			arglist[i++] = q;
			argvec = offset;
		}else
			offset++;
	}
	arglist[i] = NULL;
	*job = newjob;

#ifdef DEBUG

	printf("enqcmd argnum %d\n",enqcmd.argnum);
	for(i = 0;i < enqcmd.argnum; i++)
		printf("parse enqcmd:%s\n",arglist[i]);

#endif

	/*ÏòµÈ´ý¶ÓÁÐÖÐÔö¼ÓÐÂµÄ×÷Òµ*/
	newnode = (struct waitqueue*)malloc(sizeof(struct waitqueue));
	newnode->next =NULL;
	newnode->job=newjob;

	queueNum = getQueueNum(newnode->job);
	if(head[queueNum])
	{
		for(p=head[queueNum];p->next != NULL; p=p->next);
		p->next =newnode;
	}else{
		head[queueNum]=newnode;
	}

	/*Îª×÷Òµ´´½¨½ø³Ì*/
	if((pid=fork())<0)
		error_sys("enq fork failed");

	if(pid==0){
		newjob->pid =getpid();
		/*×èÈû×Ó½ø³Ì,µÈµÈÖ´ÐÐ*/
		raise(SIGSTOP);
#ifdef DEBUG

		printf("begin running\n");
		for(i=0;arglist[i]!=NULL;i++)
			printf("arglist %s\n",arglist[i]);
#endif

		/*¸´ÖÆÎÄ¼þÃèÊö·ûµ½±ê×¼Êä³ö*/
		//dup2(3,1);
		//write(STDIN_FILENO,"running",20);
		/* Ö´ÐÐÃüÁî */
		if(execv(arglist[0],arglist)<0)
			printf("exec failed\n");
		exit(1);
	}else{
		newjob->pid=pid;
		waitpid(pid,NULL,0);
	}
}

void do_deq(struct jobcmd deqcmd)
{
	int deqid,i;
	struct waitqueue *p,*prev,*select,*selectprev;
	deqid=atoi(deqcmd.data);

	printf("deq jid %d\n",deqid);

	/*current jodid==deqid,ÖÕÖ¹µ±Ç°×÷Òµ*/
	if (current && current->job->jid ==deqid){
		printf("teminate current job\n");
		kill(current->job->pid,SIGKILL);
		Free(current);
		current=NULL;
		timeCount = 0;
	}
	else{ /* »òÕßÔÚµÈ´ý¶ÓÁÐÖÐ²éÕÒdeqid */
		select=NULL;
		selectprev=NULL;

		for(i=0;i<3;i++){
			if(head[i]){
				for(prev=NULL,p=head[i];p!=NULL;prev=p,p=p->next)
					if(p->job->jid==deqid){
						select=p;
						selectprev=prev;
						break;
					}
				if(select){
					if(selectprev==NULL){
						head[i] = head[i]->next;
					}
					else{
						selectprev->next=select->next;
					}
					break;
				}
			}
		}
		if(select){
			kill(select->job->pid,SIGKILL);
			free(select);
			select=NULL;
		}
		else{
			printf("not found jid:%d\n",deqid );
		}
	}
}

void do_stat(struct jobcmd statcmd)
{
	struct waitqueue *p;
	char timebuf[BUFLEN];
	int i;
	struct jobinfo endjob;
	/*
	*´òÓ¡ËùÓÐ×÷ÒµµÄÍ³¼ÆÐÅÏ¢:
	*1.×÷ÒµID
	*2.½ø³ÌID
	*3.×÷ÒµËùÓÐÕß
	*4.×÷ÒµÔËÐÐÊ±¼ä
	*5.×÷ÒµµÈ´ýÊ±¼ä
	*6.×÷Òµ´´½¨Ê±¼ä
	*7.×÷Òµ×´Ì¬
	*/

	if((pipe_fd = open("D:\\tmp\\myfifo", O_WRONLY)) < 0)
			error_sys("stat open myfifo failed");
	
	//printf("JOBID\tPID\tOWNER\tRUNTIME\tWAITTIME\tCREATTIME\t\tSTATE\tDEFPRI\tCURPRI\n");

	if(current){
		/*strcpy(timebuf,ctime(&(current->job->create_time)));
		timebuf[strlen(timebuf)-1]='\0';
		printf("%d\t%d\t%d\t%d\t%d\t%s\t%s\t%d\t%d\n",
			current->job->jid,
			current->job->pid,
			current->job->ownerid,
			current->job->run_time,
			current->job->wait_time,
			timebuf,"RUNNING",
			current->job->defpri,
			current->job->curpri);
		*/
		if(write(pipe_fd, current->job, sizeof(struct jobinfo)) < 0)
			error_sys("stat write current myfifo failed");
	}
	for(i=0;i<3;i++){
		for(p=head[i];p!=NULL;p=p->next){
			/*strcpy(timebuf,ctime(&(p->job->create_time)));
			timebuf[strlen(timebuf)-1]='\0';
			printf("%d\t%d\t%d\t%d\t%d\t%s\t%s\t%d\t%d\n",
				p->job->jid,
				p->job->pid,
				p->job->ownerid,
				p->job->run_time,
				p->job->wait_time,
				timebuf,
				"READY",
				p->job->defpri,
				p->job->curpri);
			*/
			if(write(pipe_fd, p->job, sizeof(struct jobinfo)) < 0)
				error_sys("stat write queue myfifo failed");
		}
	}
	endjob.jid = 0;
	if(write(pipe_fd, &endjob , sizeof(struct jobinfo)) < 0)
		error_sys("stat write queue myfifo failed");
}

void executeCmd(struct jobcmd cmd){
	struct jobinfo *newjob;
	switch(cmd.type){
		case ENQ:
			#ifdef DEBUG
			printf("ENQ_before\n");
			print_jobInfoInQueue();
			#endif

			do_enq(&newjob,cmd);
			if((!current) || (newjob->curpri) > (current->job->curpri)){
				scheduler();
			}

			#ifdef DEBUG
			printf("ENQ\n");
			print_jobInfoInQueue();
			#endif
			break;
		case DEQ:
			#ifdef DEBUG
			printf("DEQ_before\n");
			print_jobInfoInQueue();
			#endif
			do_deq(cmd);
			if(!current){
				scheduler();
			}
			#ifdef DEBUG
			printf("DEQ\n");
			print_jobInfoInQueue();	
			#endif		
			break;
		case STAT:
			#ifdef DEBUG
			printf("STAT_before\n");
			print_jobInfoInQueue();
			printf("STAT\n");
			#endif
			do_stat(cmd);
			break;
		default:
			break;
		}
}

void SIGCHLDhandler(int sig, siginfo_t *sip, void* noused){

	#ifdef DEBUG
	printf("send SIGCHLD before\n");
	print_jobInfoInQueue();
	#endif
	if (sip->si_code == CLD_STOPPED){
		printf("child stopped\n");
	}else if(sip->si_code == CLD_EXITED){
		current->job->state = DONE;
		timeCount = 0;
		scheduler();
		printf("normal termation\n");
	}else if (sip->si_code == CLD_DUMPED){
		timeCount = 0;
		printf("abnormal termation\n");
	}
	#ifdef DEBUG
	print_jobInfoInQueue();
	#endif
	return;
}

void timerHandler(){
	printf("TIME:%d\n",timeCount);
	if(timeCount>0){
		timeCount--;
		return;
	}else{
		scheduler();
		return;
	}
}

int main()
{
	struct stat statbuf;
	struct timeval interval;
	struct itimerval new;
	struct jobcmd cmd;
	int  count = 0;

	if(stat("D:\\tmp\\server",&statbuf)==0){
		/* Èç¹ûFIFOÎÄ¼þ´æÔÚ,É¾µô */
		if(remove("D:\\tmp\\server")<0)
			error_sys("remove failed");
	}
	if(stat("D:\\tmp\\myfifo",&statbuf)==0){
		/* Èç¹ûFIFOÎÄ¼þ´æÔÚ,É¾µô */
		if(remove("D:\\tmp\\myfifo")<0)
			error_sys("remove failed");
	}

	if(mkfifo("D:\\tmp\\server",0666)<0)
		error_sys("mkfifo failed");
	if(mkfifo("D:\\tmp\\myfifo",0666)<0)
		error_sys("mkfifo myfifo failed");

	/* ÔÚ·Ç×èÈûÄ£Ê½ÏÂ´ò¿ªFIFO */
	if((fifo=open("D:\\tmp\\server",O_RDONLY|O_NONBLOCK))<0)
		error_sys("open fifo failed");

	struct sigaction action;
    action.sa_sigaction = SIGCHLDhandler;
    sigfillset(&action.sa_mask);
    action.sa_flags = SA_SIGINFO;
    sigaction(SIGCHLD, &action, NULL);
    
	//set Timer
	interval.tv_usec=(long)(1000000/CLOCK_PER_SEC);
	interval.tv_sec=0;
	new.it_value=interval;
	new.it_interval = interval;
	setitimer(ITIMER_REAL,&new,NULL);
	signal(SIGALRM,timerHandler);

	while(siginfo==1){
		bzero(&cmd,DATALEN);
		count=read(fifo,&cmd,DATALEN);
		if(count<0){
			error_sys("read fifo failed");
		}
		else if(count>0){
			signal(SIGALRM,SIG_IGN);
			executeCmd(cmd);
			signal(SIGALRM,timerHandler);
		}
		usleep(10000);
		#ifdef MYDEBUG
			//print_jobInfoInQueue();
		#endif
	};

	close(fifo);
	close(pipe_fd);
	close(globalfd);
	return 0;
}