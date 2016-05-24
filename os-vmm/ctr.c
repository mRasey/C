#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <string.h>
#include <signal.h>
#include <fcntl.h>
#include "vmm.h"

/* 页表 */
PageTableItem pageTable;
/* 实存空间 */
BYTE actMem[ACTUAL_MEMORY_SIZE];
/* 用文件模拟辅存空间 */
FILE *ptr_auxMem;
/* 物理块使用标识 */
BOOL blockStatus[BLOCK_SUM];
/* 访存请求 */
Ptr_MemoryAccessRequest ptr_memAccReq;

/* 产生访存请求 */
void do_request()
{
	/* 随机产生请求地址 */
	ptr_memAccReq->virAddr = random() % PAGE_MEMORY_SIZE;
	/* 随机产生请求类型 */
	switch (random() % 3)
	{
		case 0: //读请求
		{
			ptr_memAccReq->reqType = REQUEST_READ;
			printf("产生请求：\n地址：%u\t类型：读取\n", ptr_memAccReq->virAddr);
			break;
		}
		case 1: //写请求
		{
			ptr_memAccReq->reqType = REQUEST_WRITE;
			/* 随机产生待写入的值 */
			ptr_memAccReq->value = random() % 0xFFu;
			printf("产生请求：\n地址：%u\t类型：写入\t值：%c\n", ptr_memAccReq->virAddr, ptr_memAccReq->value);
			break;
		}
		case 2:
		{
			ptr_memAccReq->reqType = REQUEST_EXECUTE;
			printf("产生请求：\n地址：%u\t类型：执行\n", ptr_memAccReq->virAddr);
			break;
		}
		default:
			break;
	}	
	/* randomly generate request process*/
	ptr_memAccReq->process = random() % 5;
	printf("请求进程:%u\n",ptr_memAccReq->process);
}

void do_request_manual(MemoryAccessRequestType reqType,
	unsigned long virAddr,BYTE value,/*unsigned */int process)
{
	ptr_memAccReq->virAddr = virAddr;
	ptr_memAccReq->reqType = reqType;
	switch (reqType)
	{
		case 0: //读请求
		{
			printf("产生请求：\n地址：%u\t类型：读取\n", ptr_memAccReq->virAddr);
			break;
		}
		case 1: //写请求
		{
			ptr_memAccReq->value = value;
			printf("产生请求：\n地址：%u\t类型：写入\t值：%02X\n", ptr_memAccReq->virAddr, ptr_memAccReq->value);
			break;
		}
		case 2:
		{
			printf("产生请求：\n地址：%u\t类型：执行\n", ptr_memAccReq->virAddr);
			break;
		}
		default:
			break;
	}
	/* randomly generate request process*/
	ptr_memAccReq->process = process;
	printf("请求进程:%u\n",ptr_memAccReq->process);
}

/* 获取页面保护类型字符串 */
char *get_proType_str(char *str, BYTE type)
{
	if (type & READABLE)
		str[0] = 'r';
	else
		str[0] = '-';
	if (type & WRITABLE)
		str[1] = 'w';
	else
		str[1] = '-';
	if (type & EXECUTABLE)
		str[2] = 'x';
	else
		str[2] = '-';
	str[3] = '\0';
	return str;
}

char *get_accessibility_str(char *str, BYTE accessibility)
{
	if (accessibility & ZERO)
		str[0] = '0';
	else
		str[0] = '-';
	if (accessibility & ONE)
		str[1] = '1';
	else
		str[1] = '-';
	if (accessibility & TWO)
		str[2] = '2';
	else
		str[2] = '-';
	if (accessibility & THREE)
		str[3] = '3';
	else
		str[3] = '-';
	if (accessibility & FOUR)
		str[4] = '4';
	else
		str[4] = '-';
	str[5] = '\0';
	return str;
}



int main(int argc, char* argv[])
{
	int vmm_fd;
	int ctr_fd;
	char result[100] = "";
	int count;
	char input[10];
	char str[4];
	char str2[6];
	int i;
	ptr_memAccReq = (Ptr_MemoryAccessRequest) malloc(sizeof(MemoryAccessRequest));
	MemoryAccessRequestType maqt;
	char a[100];
	unsigned long b;
	BYTE c;
	unsigned int d;
	// if(stat("/tmp/to_vmm",&statbuf)==0){
	// 	if(remove("/tmp/to_vmm")<0)
	// 		printf("remove to_vmm failed\n");
	// }
	// if(stat("/tmp/to_ctr",&statbuf)==0){
	// 	if(remove("/tmp/to_ctr")<0)
	// 		printf("remove to_ctr failed\n");
	// }
	if((vmm_fd=open("/tmp/to_vmm",O_RDWR|O_NONBLOCK))<0)
			printf("open to_vmm failed\n");
	if((ctr_fd = open("/tmp/to_ctr", O_RDONLY|O_NONBLOCK)) < 0 )
			printf("open to_ctr failed\n");
	while(1)
	{
		// printf("qwerqewr\n");
		scanf("%s", input);
		// puts(input);
		// printf("%s\n", input);
		if(strcmp(input, "random") == 0){
			//随机产生请求
			do_request();
			if(write(vmm_fd,ptr_memAccReq,sizeof(MemoryAccessRequest))<0)
				printf("to_vmm write failed\n");
			i = 0;
			while((count = read(ctr_fd, &pageTable, sizeof(PageTableItem))) < 0);
			printf("页号\t块号\t装入\t修改\t保护\t辅存\t可访问性\n");
			do{
				if(count == 0)
					continue;
				else if(count > 0){
					printf("%u\t%u\t%u\t%u\t%s\t%u\t%s\n", i++, pageTable.blockNum, pageTable.filled, 
							pageTable.edited, get_proType_str(str, pageTable.proType),pageTable.auxAddr,
							get_accessibility_str(str2,pageTable.accessiblePro));
				}
			}while((count = read(ctr_fd, &pageTable, sizeof(PageTableItem))) >= 0);
		}
		else if(strcmp(input, "input") == 0){
			scanf("%s %ld %c %u", a, &b, &c, &d);
			if(strcmp(a, "REQUEST_READ") == 0)
				maqt = REQUEST_READ;
			else if(strcmp(a, "REQUEST_WRITE") == 0)
				maqt = REQUEST_WRITE;
			else if(strcmp(a, "REQUEST_EXECUTE") == 0)
				maqt = REQUEST_EXECUTE;
			do_request_manual(maqt, b, c, d);
			if(write(vmm_fd,ptr_memAccReq,sizeof(MemoryAccessRequest))<0)
				printf("to_vmm write failed\n");
			i = 0;
			while((count = read(ctr_fd, &pageTable, sizeof(PageTableItem))) < 0);
			printf("页号\t块号\t装入\t修改\t保护\t辅存\t可访问性\n");
			do{
				if(count == 0)
					continue;
				else if(count > 0){
					printf("%u\t%u\t%u\t%u\t%s\t%u\t%s\n", i++, pageTable.blockNum, pageTable.filled, 
							pageTable.edited, get_proType_str(str, pageTable.proType),pageTable.auxAddr,
							get_accessibility_str(str2,pageTable.accessiblePro));
				}
			}while((count = read(ctr_fd, &pageTable, sizeof(PageTableItem))) >= 0);
			
		}
		// if(write(vmm_fd,&ptr_memAccReq,sizeof(MemoryAccessRequest))<0)
		// 	printf("to_vmm write failed\n");
		// i = 0;
		// while((count = read(ctr_fd, &pageTable, sizeof(PageTableItem))) < 0);
		// printf("页号\t块号\t装入\t修改\t保护\t计数\t辅存\t可访问性\n");
		// do{
		// 	if(count == 0)
		// 		continue;
		// 	else if(count > 0){
		// 		printf("%u\t%u\t%u\t%u\t%s\t%u\t%u\t%s\n", i++, pageTable.blockNum, pageTable.filled, 
		// 				pageTable.edited, get_proType_str(str, pageTable.proType), 
		// 				pageTable.count, pageTable.auxAddr,
		// 				get_accessibility_str(str2,pageTable.accessiblePro));
		// 	}
		// }while((count = read(ctr_fd, &pageTable, sizeof(PageTableItem))) >= 0);
		
		// printf("end\n");
	}

	return 0;
}
