#ifndef VMM_H
#define VMM_H

#ifndef DEBUG
#define DEBUG
#endif
#undef DEBUG


/* 模拟辅存的文件路径 */
#define AUXILIARY_MEMORY "vmm_auxMem.txt"
/* 模拟二级页表的文件路径*/
#define PAGETABLE_MEMORY "page.txt"
/* 页面大小（字节）*/
#define PAGE_SIZE 4
/* 一级页表数*/
#define PAGE_DIRECTORY_SUM 4
/* 虚存空间大小（字节） */
#define VIRTUAL_MEMORY_SIZE (PAGE_DIRECTORY_SUM*64 * PAGE_SIZE)
/* 一份二级页表空间大小*/
#define PAGE_MEMORY_SIZE (64*PAGE_SIZE)
/* 实存空间大小（字节） */
#define ACTUAL_MEMORY_SIZE (32 * PAGE_SIZE)
/* 一份二级页表虚页数*/
#define PAGE_SUM (PAGE_MEMORY_SIZE / PAGE_SIZE)
/* 总物理块数 */
#define BLOCK_SUM (ACTUAL_MEMORY_SIZE / PAGE_SIZE)


/* 可读标识位 */
#define READABLE 0x01u
/* 可写标识位 */
#define WRITABLE 0x02u
/* 可执行标识位 */
#define EXECUTABLE 0x04u

#define FOUR 0x10u
#define THREE 0x08u
#define TWO 0x04u
#define ONE 0x02u
#define ZERO 0x01u


/* 定义字节类型 */
#define BYTE  char

typedef enum {
	TRUE = 1, FALSE = 0
} BOOL;



/* 页表项 */
typedef struct
{
	unsigned int pageNum;
	unsigned int blockNum; //物理块号
	BOOL filled; //页面装入特征位
	BYTE proType; //页面保护类型
	BYTE accessiblePro; //
	BOOL edited; //页面修改标识
	unsigned long auxAddr; //外存地址
} PageTableItem, *Ptr_PageTableItem;


/* 页目录项(一级页表)*/
typedef struct
{
    unsigned int PageDirectoryNum; //一级页表号
    BOOL filled;
    unsigned long pageAddr; //二级页表地址
} PageDirectoryItem, *Ptr_PageDirectoryItem;

/* 内存块信息*/
typedef struct
{
	BOOL filled; // 是否有装入
	unsigned int pageItemNum;// 装入的二级页表块号
	unsigned long count; //页面使用计数器
}BlockStatus;

/* 访存请求类型 */
typedef enum {
	REQUEST_READ,
	REQUEST_WRITE,
	REQUEST_EXECUTE
} MemoryAccessRequestType;

/* 访存请求 */
typedef struct
{
	MemoryAccessRequestType reqType; //访存请求类型
	unsigned long virAddr; //虚地址
	BYTE value; //写请求的值
	unsigned int process; //
} MemoryAccessRequest, *Ptr_MemoryAccessRequest;


/* 访存错误代码 */
typedef enum {
	ERROR_READ_DENY, //该页不可读
	ERROR_WRITE_DENY, //该页不可写
	ERROR_EXECUTE_DENY, //该页不可执行
	ERROR_INVALID_REQUEST, //非法请求类型
	ERROR_OVER_BOUNDARY, //地址越界
	ERROR_FILE_OPEN_FAILED, //文件打开失败
	ERROR_FILE_CLOSE_FAILED, //文件关闭失败
	ERROR_FILE_SEEK_FAILED, //文件指针定位失败
	ERROR_FILE_READ_FAILED, //文件读取失败
	ERROR_FILE_WRITE_FAILED, //文件写入失败
	ERROR_PROCESS_INACCESSIBLE
} ERROR_CODE;

/* 产生访存请求 */
void do_request();

/* 响应访存请求 */
void do_response();

/* 处理缺页中断 */
void do_page_fault(Ptr_PageTableItem);

/* Aging页面替换 */
void do_Aging(Ptr_PageTableItem ptr_pageTabIt);

/* 装入页面 */
void do_page_in(Ptr_PageTableItem, unsigned in);

/* 写出页面 */
void do_page_out(Ptr_PageTableItem);

/* 错误处理 */
void do_error(ERROR_CODE);

/* 打印页表相关信息 */
void do_print_info();

/* 获取页面保护类型字符串 */
char *get_proType_str(char *, BYTE);

char *get_accessibility_str(char *str, BYTE accessibility);

void do_pageTable_out(Ptr_PageDirectoryItem ptr_pageDirectoryIt);

void do_pageTable_in(Ptr_PageDirectoryItem ptr_pageDirectoryIt);

void count_handler(unsigned int pageNum);

void getPageItem(Ptr_PageTableItem ptr_pageTabIt,int number);

void writeBackPageItem(Ptr_PageTableItem ptr_pageTabIt,int number);
#endif
