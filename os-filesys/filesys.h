#ifndef FILESYS_H
#define FILESYS_H
#include<stddef.h>
#define DEVNAME "/home/wirehack/Documents/os-filesys-master/virtualUSB"                         
#define DIR_ENTRY_SIZE 32
#define SECTOR_SIZE 512   //y
#define CLUSTER_SIZE 512*4  //y                         
#define FAT_ONE_OFFSET 512                     
#define FAT_TWO_OFFSET (512+256*512)                       
#define ROOTDIR_OFFSET (512+256*512*2)                  
#define DATA_OFFSET (512+256*512*2+512*32)        

           

/*����λ����*/
#define ATTR_READONLY 0x01
#define ATTR_HIDDEN 0x02
#define ATTR_SYSTEM 0x04
#define ATTR_VLABEL 0x08
#define ATTR_SUBDIR 0x10
#define ATTR_ARCHIVE 0x20

/*ʱ������ 5��6��5 */
#define MASK_HOUR 0xf800 
#define MASK_MIN 0x07e0
#define MASK_SEC 0x001f

/*��������*/
#define MASK_YEAR 0xfe00
#define MASK_MONTH 0x01e0
#define MASK_DAY 0x001f

struct BootDescriptor_t{
	unsigned char Oem_name[9]; /*0x03-0x0a*/
	int BytesPerSector;        /*0x0b-0x0c*/
	int SectorsPerCluster;     /*0x0d*/
	int ReservedSectors;       /*0x0e-0x0f*/
	int FATs;                  /*0x10*/
	int RootDirEntries;        /*0x11-0x12*/
	int LogicSectors;          /*0x13-0x14*/
	int MediaType;             /*0x15*/
	int SectorsPerFAT;         /*0x16-0x17*/
	int SectorsPerTrack;       /*0x18-0x19*/
	int Heads;                 /*0x1a-0x1b*/
	int HiddenSectors;         /*0x1c-0x1d*/
};

struct Entry{
	unsigned char short_name[12];   /*�ֽ�0-10��11�ֽڵĶ��ļ���*/
	unsigned char long_name[27];    /*δʹ�ã�26�ֽڵĳ��ļ���*/
	unsigned short year,month,day;  /*22-23�ֽ�*/
	unsigned short hour,min,sec;    /*24-25�ֽ�*/
	unsigned short FirstCluster;    /*26-27�ֽ�*/
	unsigned int size;              /*28-31�ֽ�*/
	/*����ֵ                        11�ֽ�
	*7  6  5  4  3  2  1  0
	*N  N  A  D  V  S  H  R         Nδʹ��
	*/

	unsigned char readonly:1;
	unsigned char hidden:1;
	unsigned char system:1;
	unsigned char vlabel:1;
	unsigned char subdir:1;
	unsigned char archive:1;
};

int fd_ls();
int fd_cd(char *dir);
int fd_df(char *file_name);
int fd_cf(char *file_name,char* contain);
int fd_pwd();

void findDate(unsigned short *year,
			  unsigned short *month,
			  unsigned short *day,
			  unsigned char info[2]);

void findTime(unsigned short *hour,
			  unsigned short *min,
			  unsigned short *sec,
			  unsigned char info[2]);
int ReadFat();
int WriteFat();
void ScanBootSector();
void ScanRootEntry();
int ScanEntry(char *entryname,struct Entry *pentry,int mode);
int GetEntry(struct Entry *entry);
void FileNameFormat(unsigned char *name);
unsigned short GetFatCluster(unsigned short prev);
void ClearFatCluster(unsigned short cluster);
void clearDir(unsigned short cluster);
void get_curdir_path(char* path);

int fd;
struct BootDescriptor_t bdptor;
struct Entry *curdir = NULL;
int dirno = -1;/*����Ŀ¼�Ĳ���*/
struct Entry* fatherdir[10];
int delete_dir_mode=0;
unsigned char fatbuf[512*256];  

#endif

