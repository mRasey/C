#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <string.h>
#include <signal.h>
#include <fcntl.h>
#include "vmm.h"


/* 常驻内存二级页表 */
PageTableItem pageTable[PAGE_SUM];
/* 实存空间 */
BYTE actMem[ACTUAL_MEMORY_SIZE];
/* 用文件模拟辅存空间 */
FILE *ptr_auxMem;
/* 物理块使用标识 */
BlockStatus blockStatus[BLOCK_SUM];
/* 访存请求 */
Ptr_MemoryAccessRequest ptr_memAccReq;

/* 一级页表*/
PageDirectoryItem pageDirectoryTable[PAGE_DIRECTORY_SUM];
/* 二级页表所在文件*/
FILE *ptr_pageMem;
/* 指出当前的常驻内存二级页表的一级页表项*/
Ptr_PageDirectoryItem current_PageDirectory;

int vmm_fd;
int ctr_fd;
char string[100];//通过写入管道传给ctr

/* 初始化环境 */
void do_init()
{
	int i, j;
	srand(time(NULL));
	//初始化一级页表项
	for (i = 0; i<PAGE_DIRECTORY_SUM;i++){
        pageDirectoryTable[i].filled=FALSE;
        pageDirectoryTable[i].pageAddr=i*PAGE_SUM*sizeof(PageTableItem);
        pageDirectoryTable[i].PageDirectoryNum=i;
	}
	//初始化二级页表项
    for (j =0;j<PAGE_DIRECTORY_SUM;j++){
        for (i = 0; i < PAGE_SUM; i++)
        {
            pageTable[i].pageNum = i;
            pageTable[i].filled = FALSE;
            pageTable[i].edited = FALSE;
            /* 使用随机数设置该页的保护类型 */
            switch (rand() % 7)
            {
                case 0:
                {
                    pageTable[i].proType = READABLE;
                    break;
                }
                case 1:
                {
                    pageTable[i].proType = WRITABLE;
                    break;
                }
                case 2:
                {
                    pageTable[i].proType = EXECUTABLE;
                    break;
                }
                case 3:
                {
                    pageTable[i].proType = READABLE | WRITABLE;
                    break;
                }
                case 4:
                {
                    pageTable[i].proType = READABLE | EXECUTABLE;
                    break;
                }
                case 5:
                {
                    pageTable[i].proType = WRITABLE | EXECUTABLE;
                    break;
                }
                case 6:
                {
                    pageTable[i].proType = READABLE | WRITABLE | EXECUTABLE;
                    break;
                }
                default:
                    break;
            }

            /* set accessible process */
			pageTable[i].accessiblePro = random() % 32;

            /* 设置该页对应的辅存地址 modified*/
            pageTable[i].auxAddr = j*PAGE_MEMORY_SIZE + i * PAGE_SIZE;
        }
        //装入二级页表所在文件
        do_pageTable_out(&pageDirectoryTable[j]);
    }
    // 随机的选择一块二级页表常驻内存
    do_pageTable_in(&pageDirectoryTable[rand()%PAGE_DIRECTORY_SUM]);
    //do_pageTable_in(&pageDirectoryTable[0]);
	for (j = 0; j < BLOCK_SUM; j++)
	{
		/* 随机选择一些物理块进行页面装入 */
		if (rand()%2==0)
		{
			do_page_in(&pageTable[j], j);
			pageTable[j].blockNum = j;
			pageTable[j].filled = TRUE;
			blockStatus[j].filled=TRUE;
			blockStatus[j].pageItemNum=current_PageDirectory->PageDirectoryNum*64+j;
			blockStatus[j].count=0;
		}
		else
			blockStatus[j].filled = FALSE;
	}
}
/* 将二级页表写回文件*/
void do_pageTable_out(Ptr_PageDirectoryItem ptr_pageDirectoryIt)
{
    int writeNum;
	if (fseek(ptr_pageMem, ptr_pageDirectoryIt->pageAddr, SEEK_SET) < 0)
	{
	#ifdef DEBUG
			printf("DEBUG: auxAddr=%u\tftell=%u\n", ptr_pageTabIt, ftell(ptr_auxMem));
	#endif
		do_error(ERROR_FILE_SEEK_FAILED);
		exit(1);
	}
	if ((writeNum = fwrite(pageTable,
		sizeof(PageTableItem), PAGE_SUM, ptr_pageMem)) < PAGE_SUM)
	{
	#ifdef DEBUG
			printf("DEBUG: auxAddr=%u\tftell=%u\n", ptr_pageDirectoryIt->pageAddr, ftell(ptr_pageMem));
			printf("DEBUG: writeNum=%u\n", writeNum);
			printf("DEGUB: feof=%d\tferror=%d\n", feof(ptr_pageMem), ferror(ptr_pageMem));
	#endif
		do_error(ERROR_FILE_WRITE_FAILED);
		exit(1);
	}
	printf("二级页表写回成功：二级页表区号%d\n",ptr_pageDirectoryIt->PageDirectoryNum);
    ptr_pageDirectoryIt->filled=FALSE;
}

/* 将二级页表常驻内存*/
void do_pageTable_in(Ptr_PageDirectoryItem ptr_pageDirectoryIt)
{
    int writeNum;
	if (fseek(ptr_pageMem, ptr_pageDirectoryIt->pageAddr, SEEK_SET) < 0)
	{
	#ifdef DEBUG
			printf("DEBUG: auxAddr=%u\tftell=%u\n", ptr_pageDirectoryIt, ftell(ptr_pageMem));
	#endif
		do_error(ERROR_FILE_SEEK_FAILED);
		exit(1);
	}
	if ((writeNum = fread(pageTable,
		sizeof(PageTableItem), PAGE_SUM,ptr_pageMem)) < PAGE_SUM)
	{
	#ifdef DEBUG
			printf("DEBUG: writeNum=%u\n", writeNum);
			printf("DEGUB: feof=%d\tferror=%d\n", feof(ptr_pageMem), ferror(ptr_pageMem));
	        printf("写入完成%d \n",ftell(ptr_pageMem));
	#endif
		do_error(ERROR_FILE_WRITE_FAILED);
		exit(1);
	}
	printf("二级页表装入成功：二级页表区号%d\n", ptr_pageDirectoryIt->PageDirectoryNum);
    ptr_pageDirectoryIt->filled=TRUE;
    //赋值内存中当前一级页表项
    current_PageDirectory=ptr_pageDirectoryIt;
}

/* 响应请求 */
void do_response()
{
	Ptr_PageTableItem ptr_pageTabIt;
	unsigned int pageNum, offAddr,directoryNum;
	unsigned int actAddr;
	unsigned int processNum;
	BYTE accessibility;
	printf("%d %ld %c %u\n", ptr_memAccReq->reqType, 
							ptr_memAccReq->virAddr, 
							ptr_memAccReq->value, 
							ptr_memAccReq->process);

	/* 检查地址是否越界 */
	if (ptr_memAccReq->virAddr < 0 || ptr_memAccReq->virAddr >= VIRTUAL_MEMORY_SIZE)
	{
		do_error(ERROR_OVER_BOUNDARY);
		return;
	}

	/* 计算一级页目录号，二级页号和页内偏移值 */
	directoryNum = ptr_memAccReq->virAddr /PAGE_MEMORY_SIZE;
	pageNum = (ptr_memAccReq->virAddr % PAGE_MEMORY_SIZE) / PAGE_SIZE;
	offAddr = (ptr_memAccReq->virAddr % PAGE_MEMORY_SIZE) % PAGE_SIZE;
	printf("目录号为：%u\t页号为：%u\t页内偏移为：%u\n", directoryNum,pageNum, offAddr);

    if (!pageDirectoryTable[directoryNum].filled){
        //换入要访问的一级页面，先将当前一级页表写回，然后将所要访问的一级页表装入。
        do_pageTable_out(current_PageDirectory);
        do_pageTable_in(&pageDirectoryTable[directoryNum]);
    }

	/* 获取对应页表项 */
	ptr_pageTabIt = &pageTable[pageNum];

	/* 根据特征位决定是否产生缺页中断 */
	if (!ptr_pageTabIt->filled)
	{
		do_page_fault(ptr_pageTabIt);
	}

	actAddr = ptr_pageTabIt->blockNum * PAGE_SIZE + offAddr;
	printf("实地址为：%u\n", actAddr);
	
	/* check process accessibility */
	processNum = ptr_memAccReq->process;
	accessibility = ptr_pageTabIt->accessiblePro;
	if((accessibility & (1<<processNum)) == 0){
		do_error(ERROR_PROCESS_INACCESSIBLE);
		return;
	}
	/* 检查页面访问权限并处理访存请求 */
	switch (ptr_memAccReq->reqType)
	{
		case REQUEST_READ: //读请求
		{
			count_handler(ptr_pageTabIt->blockNum);
			if (!(ptr_pageTabIt->proType & READABLE)) //页面不可读
			{
				do_error(ERROR_READ_DENY);
				return;
			}
			/* 读取实存中的内容 */
			//printf("读操作成功：值为%02X\n", actMem[actAddr]);
			printf("读操作成功：值为%c\n", actMem[actAddr]);			
			break;
		}
		case REQUEST_WRITE: //写请求
		{
			count_handler(ptr_pageTabIt->blockNum);
			if (!(ptr_pageTabIt->proType & WRITABLE)) //页面不可写
			{
				do_error(ERROR_WRITE_DENY);
				return;
			}
			/* 向实存中写入请求的内容 */
			actMem[actAddr] = ptr_memAccReq->value;
			ptr_pageTabIt->edited = TRUE;
			printf("写操作成功\n");
			break;
		}
		case REQUEST_EXECUTE: //执行请求
		{
			count_handler(ptr_pageTabIt->blockNum);
			if (!(ptr_pageTabIt->proType & EXECUTABLE)) //页面不可执行
			{
				do_error(ERROR_EXECUTE_DENY);
				return;
			}
			printf("执行成功\n");
			break;
		}
		default: //非法请求类型
		{
			do_error(ERROR_INVALID_REQUEST);
			return;
		}
	}
}

void count_handler(unsigned int pageNum){
	unsigned int i;
	for (i = 0; i < BLOCK_SUM; i++){
        if (blockStatus[i].filled)
            blockStatus[i].count=blockStatus[i].count>>1;
	}
	blockStatus[pageNum].count+=0x80000000;
}
/* 处理缺页中断 */
void do_page_fault(Ptr_PageTableItem ptr_pageTabIt)
{
	unsigned int i;
	printf("产生缺页中断，开始进行调页...\n");
	for (i = 0; i < BLOCK_SUM; i++)
	{
		if (!blockStatus[i].filled)
		{
			/* 读辅存内容，写入到实存 */
			do_page_in(ptr_pageTabIt, i);

			/* 更新页表内容 */
			ptr_pageTabIt->blockNum = i;
			ptr_pageTabIt->filled = TRUE;
			ptr_pageTabIt->edited = FALSE;

			blockStatus[i].filled= TRUE;
			blockStatus[i].pageItemNum=current_PageDirectory->PageDirectoryNum*64+ptr_pageTabIt->pageNum;
			blockStatus[i].count=0;
			return;
		}
	}
	/* 没有空闲物理块，进行页面替换 */
	do_Aging(ptr_pageTabIt);
}
/* 根据Aging算法进行页面替换 */
void do_Aging(Ptr_PageTableItem ptr_pageTabIt)
{
	unsigned int i, page;
	unsigned long min;
	PageTableItem target;
	printf("没有空闲物理块，开始进行Aging页面替换...\n");
	for (i = 0, min = 0xFFFFFFFF, page = 0; i < BLOCK_SUM; i++)
	{
		if (blockStatus[i].count < min&&blockStatus[i].filled==TRUE)
		{
			min = blockStatus[i].count;
			page = i;
		}
	}
	printf("选择第%u页进行替换\n", page);
	//获取当前要替换项
	getPageItem(&target,blockStatus[page].pageItemNum);
	if (target.edited)
	{
		/* 页面内容有修改，需要写回至辅存 */
		printf("该页内容有修改，写回至辅存\n");
		do_page_out(&target);
	}
	target.filled = FALSE;
    writeBackPageItem(&target,blockStatus[page].pageItemNum);

	/* 读辅存内容，写入到实存 */
	do_page_in(ptr_pageTabIt, page);

	/* 更新页表内容 */
	ptr_pageTabIt->blockNum = page;
	ptr_pageTabIt->filled = TRUE;
	ptr_pageTabIt->edited = FALSE;
    blockStatus[page].filled = TRUE;
    blockStatus[page].pageItemNum=current_PageDirectory->PageDirectoryNum*64+ptr_pageTabIt->pageNum;
	blockStatus[page].count=0;
	printf("页面替换成功\n");
}
/* 获取选择的页表项*/
void getPageItem(Ptr_PageTableItem ptr_pageTabIt,int number){
	if (fseek(ptr_pageMem, number*sizeof(PageTableItem), SEEK_SET) < 0)
	{
		do_error(ERROR_FILE_SEEK_FAILED);
		exit(1);
	}
	if (fread(ptr_pageTabIt,
		sizeof(PageTableItem), 1,ptr_pageMem)< 1)
	{
		do_error(ERROR_FILE_WRITE_FAILED);
		exit(1);
	}
}
/* 写回页表项*/
void writeBackPageItem(Ptr_PageTableItem ptr_pageTabIt,int number){
	if (fseek(ptr_pageMem, number*sizeof(PageTableItem), SEEK_SET) < 0)
	{
		do_error(ERROR_FILE_SEEK_FAILED);
		exit(1);
	}
	if (fwrite(ptr_pageTabIt,
		sizeof(PageTableItem), 1,ptr_pageMem)< 1)
	{
		do_error(ERROR_FILE_WRITE_FAILED);
		exit(1);
	}
}

/* 将辅存内容写入实存 */
void do_page_in(Ptr_PageTableItem ptr_pageTabIt, unsigned int blockNum)
{
	unsigned int readNum;
	if (fseek(ptr_auxMem, ptr_pageTabIt->auxAddr, SEEK_SET) < 0)
	{
	#ifdef DEBUG
			printf("DEBUG: auxAddr=%u\tftell=%u\n", ptr_pageTabIt->auxAddr, ftell(ptr_auxMem));
	#endif
		do_error(ERROR_FILE_SEEK_FAILED);
		exit(1);
	}
	if ((readNum = fread(actMem + blockNum * PAGE_SIZE,
		sizeof(BYTE), PAGE_SIZE, ptr_auxMem)) < PAGE_SIZE)
	{
	#ifdef DEBUG
			printf("DEBUG: auxAddr=%u\tftell=%u\n", ptr_pageTabIt->auxAddr, ftell(ptr_auxMem));
			printf("DEBUG: blockNum=%u\treadNum=%u\n", blockNum, readNum);
			printf("DEGUB: feof=%d\tferror=%d\n", feof(ptr_auxMem), ferror(ptr_auxMem));
	#endif
		do_error(ERROR_FILE_READ_FAILED);
		exit(1);
	}
	printf("调页成功：辅存地址%u-->>物理块%u\n", ptr_pageTabIt->auxAddr, blockNum);
}

/* 将被替换页面的内容写回辅存 */
void do_page_out(Ptr_PageTableItem ptr_pageTabIt)
{
	unsigned int writeNum;
	if (fseek(ptr_auxMem, ptr_pageTabIt->auxAddr, SEEK_SET) < 0)
	{
	#ifdef DEBUG
			printf("DEBUG: auxAddr=%u\tftell=%u\n", ptr_pageTabIt, ftell(ptr_auxMem));
	#endif
		do_error(ERROR_FILE_SEEK_FAILED);
		exit(1);
	}
	if ((writeNum = fwrite(actMem + ptr_pageTabIt->blockNum * PAGE_SIZE,
		sizeof(BYTE), PAGE_SIZE, ptr_auxMem)) < PAGE_SIZE)
	{
	#ifdef DEBUG
			printf("DEBUG: auxAddr=%u\tftell=%u\n", ptr_pageTabIt->auxAddr, ftell(ptr_auxMem));
			printf("DEBUG: writeNum=%u\n", writeNum);
			printf("DEGUB: feof=%d\tferror=%d\n", feof(ptr_auxMem), ferror(ptr_auxMem));
	#endif
		do_error(ERROR_FILE_WRITE_FAILED);
		exit(1);
	}
	printf("写回成功：物理块%u-->>辅存地址%03X\n", ptr_pageTabIt->auxAddr, ptr_pageTabIt->blockNum);
}

/* 错误处理 */
void do_error(ERROR_CODE code)
{
	switch (code)
	{
		case ERROR_READ_DENY:
		{
			printf("访存失败：该地址内容不可读\n");
			break;
		}
		case ERROR_WRITE_DENY:
		{
			printf("访存失败：该地址内容不可写\n");
			break;
		}
		case ERROR_EXECUTE_DENY:
		{
			printf("访存失败：该地址内容不可执行\n");
			break;
		}
		case ERROR_INVALID_REQUEST:
		{
			printf("访存失败：非法访存请求\n");
			break;
		}
		case ERROR_OVER_BOUNDARY:
		{
			printf("访存失败：地址越界\n");
			break;
		}
		case ERROR_FILE_OPEN_FAILED:
		{
			printf("系统错误：打开文件失败\n");
			break;
		}
		case ERROR_FILE_CLOSE_FAILED:
		{
			printf("系统错误：关闭文件失败\n");
			break;
		}
		case ERROR_FILE_SEEK_FAILED:
		{
			printf("系统错误：文件指针定位失败\n");
			break;
		}
		case ERROR_FILE_READ_FAILED:
		{
			printf("系统错误：读取文件失败\n");
			break;
		}
		case ERROR_FILE_WRITE_FAILED:
		{
			printf("系统错误：写入文件失败\n");
			break;
		}
		case ERROR_PROCESS_INACCESSIBLE:
		{
			printf("process inaccessible\n");
			break;
		}
		default:
		{
			printf("未知错误：没有这个错误代码\n");
		}
	}
}
void do_print_block(){
	int i;
	int directoryNum,pageNum;
 	printf("物理块号\t装入\t计数\t目录号\t页号\t内容\n");
 	for (i = 0; i < BLOCK_SUM; ++i)
 	{
 		directoryNum=blockStatus[i].pageItemNum/64;
 		pageNum = blockStatus[i].pageItemNum%64;
		printf("%u\t\t%u\t%lu\t%u\t%u\t%c%c%c%c\n",i,blockStatus[i].filled,
			blockStatus[i].count,directoryNum,pageNum,actMem[i*4],actMem[i*4+1],actMem[i*4+2],actMem[i*4+3]);
 	}
}

void do_print_auxMem(){
	int i=0,j=0;
	char c;
	int readNum;
 	printf("辅存号\t内容\n");
	if (fseek(ptr_auxMem, 0, SEEK_SET) < 0)
	{
		do_error(ERROR_FILE_SEEK_FAILED);
		exit(1);
	}
	while((readNum = fread(&c,
		sizeof(BYTE),1, ptr_auxMem)!=0))
 	{
		printf("%d %c ",i++,c);
	j++;
	if(j==2){
		printf("\n");
	j=0;
	}
 	}
}

int main(int argc, char* argv[])
{
	char c;
	int i;
	struct stat statbuf;
	int count;
	PageTableItem* p = (PageTableItem*)malloc(sizeof(PageTableItem));

	if (!(ptr_auxMem = fopen(AUXILIARY_MEMORY, "r+")))
	{
		do_error(ERROR_FILE_OPEN_FAILED);
		exit(1);
	}
	if (!(ptr_pageMem = fopen(PAGETABLE_MEMORY, "r+")))
	{
		do_error(ERROR_FILE_OPEN_FAILED);
		exit(1);
	}

	if(stat("/tmp/to_vmm",&statbuf)==0){
		if(remove("/tmp/to_vmm")<0)
			printf("remove to_vmm failed\n");
	}

	if(stat("/tmp/to_ctr",&statbuf)==0){
		if(remove("/tmp/to_ctr")<0)
			printf("remove to_ctr failed\n");
	}

	if(mkfifo("/tmp/to_vmm",0666)<0)
		printf("mkfifo to_vmm failed\n");

	if(mkfifo("/tmp/to_ctr",0666)<0)
		printf("mkfifo to_ctr failed\n");

	if((vmm_fd=open("/tmp/to_vmm",O_RDONLY|O_NONBLOCK))<0)
		printf("open to_vmm failed\n");

	if((ctr_fd=open("/tmp/to_ctr",O_RDWR|O_NONBLOCK))<0)
		printf("open to_ctr failed\n");

	do_init();
	printf("当前页表号 %d\n", current_PageDirectory->PageDirectoryNum);	
	// do_print_info();
	ptr_memAccReq = (Ptr_MemoryAccessRequest) malloc(sizeof(MemoryAccessRequest));
	/* 在循环中模拟访存请求与处理过程 */
	while (TRUE)
	{
		// do_request();
		// bzero(&ptr_memAccReq, sizeof(ptr_memAccReq))
			
		count = read(vmm_fd, ptr_memAccReq, sizeof(MemoryAccessRequest));//从管道读取指令
			
		if(count > 0){
			// printf("%d\n", count);
			do_response();
			//写回给ctr
			for (i = 0; i < PAGE_SUM; i++){
				// p = &pageTable[i];
				if(write(ctr_fd, &pageTable[i], sizeof(PageTableItem))<0)//向管道写入
					printf("to_ctr write failed\n");
			}
			do_print_block();
			//printf("按Y打印辅存，按其他键继续...\n");
		 	//if ((c = getchar()) == 'y' || c == 'Y')
				do_print_auxMem();
			printf("当前页表号 %d\n", current_PageDirectory->PageDirectoryNum);
			
		}
		// printf("123\n");

		// printf("按X退出程序，按其他键继续...\n");
		// if ((c = getchar()) == 'x' || c == 'X')
		// 	break;
		// while (c != '\n')
		// 	c = getchar();
	}

	if (fclose(ptr_auxMem) == EOF)
	{
		do_error(ERROR_FILE_CLOSE_FAILED);
		exit(1);
	}
	return (0);
}
