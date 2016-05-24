#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ipc.h>
#include <fcntl.h>
#include <time.h>
#include "job.h"

/* 
 * 命令语法格式
 *     stat
 */
void usage()
{
	printf("Usage: stat\n");		
}

int main(int argc,char *argv[])
{
	struct jobcmd statcmd;
	int fd;
	char* state;
	struct jobinfo jf;
	int count;
	
	int pipe_fd = -1;
	int res = 0;
	char timebuf[BUFLEN];


	
	if(argc!=1)
	{
		usage();
		return 1;
	}

	statcmd.type=STAT;
	statcmd.defpri=0;
	statcmd.owner=getuid();
	statcmd.argnum=0;

    #ifdef DEBUG
	printf("statcmd cmdtype\t%d\n"
			"statcmd owner\t%d\n",
			statcmd.type,statcmd.owner);

    #endif 

	if((fd=open("D:\\tmp\\server",O_RDWR|O_NONBLOCK))<0)
		error_sys("stat open fifo failed");

	if(write(fd,&statcmd,DATALEN)<0)
		error_sys("stat write failed");

	close(fd);

	if((pipe_fd = open("D:\\tmp\\myfifo", O_RDONLY|O_NONBLOCK)) < 0 )/*¶ÁÈ¡Ê§°Ü*/
		error_sys("open myfifo failed");
	
	bzero(&jf, sizeof(struct jobinfo));
	while((count = read(pipe_fd, &jf, sizeof(struct jobinfo))) < 0);

	printf("JOBID\tPID\tOWNER\tRUNTIME\tWAITTIME\tCREATTIME\t\tSTATE\tDEFPRI\tCURPRI\n");
	do{
		if(count == 0){
			continue;
		}
		if(jf.jid == 0){
			break;
		}
		if(jf.state == 0)
			state = "READY";
		else if(jf.state == 1)
			state = "RUNNING";
		else
			state = "DONE";
		strcpy(timebuf,ctime(&(jf.create_time)));
		timebuf[strlen(timebuf)-1]='\0';
		printf("%d\t%d\t%d\t%d\t%d\t%s\t%s\t%d\t%d\n",
			jf.jid,
			jf.pid,
			jf.ownerid,
			jf.run_time,
			jf.wait_time,
			timebuf,
			state,
			jf.defpri,
			jf.curpri);
		bzero(&jf, sizeof(struct jobinfo));
	}while((count = read(pipe_fd, &jf, sizeof(struct jobinfo))) >= 0);

	close(pipe_fd);
	return 0;
}
