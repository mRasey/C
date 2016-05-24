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


/* ��פ�ڴ����ҳ�� */
PageTableItem pageTable[PAGE_SUM];
/* ʵ��ռ� */
BYTE actMem[ACTUAL_MEMORY_SIZE];
/* ���ļ�ģ�⸨��ռ� */
FILE *ptr_auxMem;
/* �����ʹ�ñ�ʶ */
BlockStatus blockStatus[BLOCK_SUM];
/* �ô����� */
Ptr_MemoryAccessRequest ptr_memAccReq;

/* һ��ҳ��*/
PageDirectoryItem pageDirectoryTable[PAGE_DIRECTORY_SUM];
/* ����ҳ�������ļ�*/
FILE *ptr_pageMem;
/* ָ����ǰ�ĳ�פ�ڴ����ҳ���һ��ҳ����*/
Ptr_PageDirectoryItem current_PageDirectory;

int vmm_fd;
int ctr_fd;
char string[100];//ͨ��д��ܵ�����ctr

/* ��ʼ������ */
void do_init()
{
	int i, j;
	srand(time(NULL));
	//��ʼ��һ��ҳ����
	for (i = 0; i<PAGE_DIRECTORY_SUM;i++){
        pageDirectoryTable[i].filled=FALSE;
        pageDirectoryTable[i].pageAddr=i*PAGE_SUM*sizeof(PageTableItem);
        pageDirectoryTable[i].PageDirectoryNum=i;
	}
	//��ʼ������ҳ����
    for (j =0;j<PAGE_DIRECTORY_SUM;j++){
        for (i = 0; i < PAGE_SUM; i++)
        {
            pageTable[i].pageNum = i;
            pageTable[i].filled = FALSE;
            pageTable[i].edited = FALSE;
            /* ʹ����������ø�ҳ�ı������� */
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

            /* ���ø�ҳ��Ӧ�ĸ����ַ modified*/
            pageTable[i].auxAddr = j*PAGE_MEMORY_SIZE + i * PAGE_SIZE;
        }
        //װ�����ҳ�������ļ�
        do_pageTable_out(&pageDirectoryTable[j]);
    }
    // �����ѡ��һ�����ҳ��פ�ڴ�
    do_pageTable_in(&pageDirectoryTable[rand()%PAGE_DIRECTORY_SUM]);
    //do_pageTable_in(&pageDirectoryTable[0]);
	for (j = 0; j < BLOCK_SUM; j++)
	{
		/* ���ѡ��һЩ��������ҳ��װ�� */
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
/* ������ҳ��д���ļ�*/
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
	printf("����ҳ��д�سɹ�������ҳ������%d\n",ptr_pageDirectoryIt->PageDirectoryNum);
    ptr_pageDirectoryIt->filled=FALSE;
}

/* ������ҳ��פ�ڴ�*/
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
	        printf("д�����%d \n",ftell(ptr_pageMem));
	#endif
		do_error(ERROR_FILE_WRITE_FAILED);
		exit(1);
	}
	printf("����ҳ��װ��ɹ�������ҳ������%d\n", ptr_pageDirectoryIt->PageDirectoryNum);
    ptr_pageDirectoryIt->filled=TRUE;
    //��ֵ�ڴ��е�ǰһ��ҳ����
    current_PageDirectory=ptr_pageDirectoryIt;
}

/* ��Ӧ���� */
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

	/* ����ַ�Ƿ�Խ�� */
	if (ptr_memAccReq->virAddr < 0 || ptr_memAccReq->virAddr >= VIRTUAL_MEMORY_SIZE)
	{
		do_error(ERROR_OVER_BOUNDARY);
		return;
	}

	/* ����һ��ҳĿ¼�ţ�����ҳ�ź�ҳ��ƫ��ֵ */
	directoryNum = ptr_memAccReq->virAddr /PAGE_MEMORY_SIZE;
	pageNum = (ptr_memAccReq->virAddr % PAGE_MEMORY_SIZE) / PAGE_SIZE;
	offAddr = (ptr_memAccReq->virAddr % PAGE_MEMORY_SIZE) % PAGE_SIZE;
	printf("Ŀ¼��Ϊ��%u\tҳ��Ϊ��%u\tҳ��ƫ��Ϊ��%u\n", directoryNum,pageNum, offAddr);

    if (!pageDirectoryTable[directoryNum].filled){
        //����Ҫ���ʵ�һ��ҳ�棬�Ƚ���ǰһ��ҳ��д�أ�Ȼ����Ҫ���ʵ�һ��ҳ��װ�롣
        do_pageTable_out(current_PageDirectory);
        do_pageTable_in(&pageDirectoryTable[directoryNum]);
    }

	/* ��ȡ��Ӧҳ���� */
	ptr_pageTabIt = &pageTable[pageNum];

	/* ��������λ�����Ƿ����ȱҳ�ж� */
	if (!ptr_pageTabIt->filled)
	{
		do_page_fault(ptr_pageTabIt);
	}

	actAddr = ptr_pageTabIt->blockNum * PAGE_SIZE + offAddr;
	printf("ʵ��ַΪ��%u\n", actAddr);
	
	/* check process accessibility */
	processNum = ptr_memAccReq->process;
	accessibility = ptr_pageTabIt->accessiblePro;
	if((accessibility & (1<<processNum)) == 0){
		do_error(ERROR_PROCESS_INACCESSIBLE);
		return;
	}
	/* ���ҳ�����Ȩ�޲�����ô����� */
	switch (ptr_memAccReq->reqType)
	{
		case REQUEST_READ: //������
		{
			count_handler(ptr_pageTabIt->blockNum);
			if (!(ptr_pageTabIt->proType & READABLE)) //ҳ�治�ɶ�
			{
				do_error(ERROR_READ_DENY);
				return;
			}
			/* ��ȡʵ���е����� */
			//printf("�������ɹ���ֵΪ%02X\n", actMem[actAddr]);
			printf("�������ɹ���ֵΪ%c\n", actMem[actAddr]);			
			break;
		}
		case REQUEST_WRITE: //д����
		{
			count_handler(ptr_pageTabIt->blockNum);
			if (!(ptr_pageTabIt->proType & WRITABLE)) //ҳ�治��д
			{
				do_error(ERROR_WRITE_DENY);
				return;
			}
			/* ��ʵ����д����������� */
			actMem[actAddr] = ptr_memAccReq->value;
			ptr_pageTabIt->edited = TRUE;
			printf("д�����ɹ�\n");
			break;
		}
		case REQUEST_EXECUTE: //ִ������
		{
			count_handler(ptr_pageTabIt->blockNum);
			if (!(ptr_pageTabIt->proType & EXECUTABLE)) //ҳ�治��ִ��
			{
				do_error(ERROR_EXECUTE_DENY);
				return;
			}
			printf("ִ�гɹ�\n");
			break;
		}
		default: //�Ƿ���������
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
/* ����ȱҳ�ж� */
void do_page_fault(Ptr_PageTableItem ptr_pageTabIt)
{
	unsigned int i;
	printf("����ȱҳ�жϣ���ʼ���е�ҳ...\n");
	for (i = 0; i < BLOCK_SUM; i++)
	{
		if (!blockStatus[i].filled)
		{
			/* ���������ݣ�д�뵽ʵ�� */
			do_page_in(ptr_pageTabIt, i);

			/* ����ҳ������ */
			ptr_pageTabIt->blockNum = i;
			ptr_pageTabIt->filled = TRUE;
			ptr_pageTabIt->edited = FALSE;

			blockStatus[i].filled= TRUE;
			blockStatus[i].pageItemNum=current_PageDirectory->PageDirectoryNum*64+ptr_pageTabIt->pageNum;
			blockStatus[i].count=0;
			return;
		}
	}
	/* û�п�������飬����ҳ���滻 */
	do_Aging(ptr_pageTabIt);
}
/* ����Aging�㷨����ҳ���滻 */
void do_Aging(Ptr_PageTableItem ptr_pageTabIt)
{
	unsigned int i, page;
	unsigned long min;
	PageTableItem target;
	printf("û�п�������飬��ʼ����Agingҳ���滻...\n");
	for (i = 0, min = 0xFFFFFFFF, page = 0; i < BLOCK_SUM; i++)
	{
		if (blockStatus[i].count < min&&blockStatus[i].filled==TRUE)
		{
			min = blockStatus[i].count;
			page = i;
		}
	}
	printf("ѡ���%uҳ�����滻\n", page);
	//��ȡ��ǰҪ�滻��
	getPageItem(&target,blockStatus[page].pageItemNum);
	if (target.edited)
	{
		/* ҳ���������޸ģ���Ҫд�������� */
		printf("��ҳ�������޸ģ�д��������\n");
		do_page_out(&target);
	}
	target.filled = FALSE;
    writeBackPageItem(&target,blockStatus[page].pageItemNum);

	/* ���������ݣ�д�뵽ʵ�� */
	do_page_in(ptr_pageTabIt, page);

	/* ����ҳ������ */
	ptr_pageTabIt->blockNum = page;
	ptr_pageTabIt->filled = TRUE;
	ptr_pageTabIt->edited = FALSE;
    blockStatus[page].filled = TRUE;
    blockStatus[page].pageItemNum=current_PageDirectory->PageDirectoryNum*64+ptr_pageTabIt->pageNum;
	blockStatus[page].count=0;
	printf("ҳ���滻�ɹ�\n");
}
/* ��ȡѡ���ҳ����*/
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
/* д��ҳ����*/
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

/* ����������д��ʵ�� */
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
	printf("��ҳ�ɹ��������ַ%u-->>�����%u\n", ptr_pageTabIt->auxAddr, blockNum);
}

/* �����滻ҳ�������д�ظ��� */
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
	printf("д�سɹ��������%u-->>�����ַ%03X\n", ptr_pageTabIt->auxAddr, ptr_pageTabIt->blockNum);
}

/* ������ */
void do_error(ERROR_CODE code)
{
	switch (code)
	{
		case ERROR_READ_DENY:
		{
			printf("�ô�ʧ�ܣ��õ�ַ���ݲ��ɶ�\n");
			break;
		}
		case ERROR_WRITE_DENY:
		{
			printf("�ô�ʧ�ܣ��õ�ַ���ݲ���д\n");
			break;
		}
		case ERROR_EXECUTE_DENY:
		{
			printf("�ô�ʧ�ܣ��õ�ַ���ݲ���ִ��\n");
			break;
		}
		case ERROR_INVALID_REQUEST:
		{
			printf("�ô�ʧ�ܣ��Ƿ��ô�����\n");
			break;
		}
		case ERROR_OVER_BOUNDARY:
		{
			printf("�ô�ʧ�ܣ���ַԽ��\n");
			break;
		}
		case ERROR_FILE_OPEN_FAILED:
		{
			printf("ϵͳ���󣺴��ļ�ʧ��\n");
			break;
		}
		case ERROR_FILE_CLOSE_FAILED:
		{
			printf("ϵͳ���󣺹ر��ļ�ʧ��\n");
			break;
		}
		case ERROR_FILE_SEEK_FAILED:
		{
			printf("ϵͳ�����ļ�ָ�붨λʧ��\n");
			break;
		}
		case ERROR_FILE_READ_FAILED:
		{
			printf("ϵͳ���󣺶�ȡ�ļ�ʧ��\n");
			break;
		}
		case ERROR_FILE_WRITE_FAILED:
		{
			printf("ϵͳ����д���ļ�ʧ��\n");
			break;
		}
		case ERROR_PROCESS_INACCESSIBLE:
		{
			printf("process inaccessible\n");
			break;
		}
		default:
		{
			printf("δ֪����û������������\n");
		}
	}
}
void do_print_block(){
	int i;
	int directoryNum,pageNum;
 	printf("������\tװ��\t����\tĿ¼��\tҳ��\t����\n");
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
 	printf("�����\t����\n");
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
	printf("��ǰҳ��� %d\n", current_PageDirectory->PageDirectoryNum);	
	// do_print_info();
	ptr_memAccReq = (Ptr_MemoryAccessRequest) malloc(sizeof(MemoryAccessRequest));
	/* ��ѭ����ģ��ô������봦����� */
	while (TRUE)
	{
		// do_request();
		// bzero(&ptr_memAccReq, sizeof(ptr_memAccReq))
			
		count = read(vmm_fd, ptr_memAccReq, sizeof(MemoryAccessRequest));//�ӹܵ���ȡָ��
			
		if(count > 0){
			// printf("%d\n", count);
			do_response();
			//д�ظ�ctr
			for (i = 0; i < PAGE_SUM; i++){
				// p = &pageTable[i];
				if(write(ctr_fd, &pageTable[i], sizeof(PageTableItem))<0)//��ܵ�д��
					printf("to_ctr write failed\n");
			}
			do_print_block();
			//printf("��Y��ӡ���棬������������...\n");
		 	//if ((c = getchar()) == 'y' || c == 'Y')
				do_print_auxMem();
			printf("��ǰҳ��� %d\n", current_PageDirectory->PageDirectoryNum);
			
		}
		// printf("123\n");

		// printf("��X�˳����򣬰�����������...\n");
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
