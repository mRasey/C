#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include "filesys.h"


#define RevByte(low,high) ((high)<<8|(low))
#define RevWord(lowest,lower,higher,highest) ((highest)<< 24|(higher)<<16|(lower)<<8|lowest) 

/*
*���ܣ���ӡ�������¼
*/
void ScanBootSector()
{
	unsigned char buf[SECTOR_SIZE];
	int ret,i;

	if((ret = read(fd,buf,SECTOR_SIZE))<0)
		perror("read boot sector failed");
	for(i = 0; i < 8; i++)
		bdptor.Oem_name[i] = buf[i+0x03];
	bdptor.Oem_name[i] = '\0';

	bdptor.BytesPerSector = RevByte(buf[0x0b],buf[0x0c]);
	bdptor.SectorsPerCluster = buf[0x0d];
	bdptor.ReservedSectors = RevByte(buf[0x0e],buf[0x0f]);
	bdptor.FATs = buf[0x10];
	bdptor.RootDirEntries = RevByte(buf[0x11],buf[0x12]);    
	bdptor.LogicSectors = RevByte(buf[0x13],buf[0x14]);
	bdptor.MediaType = buf[0x15];
	bdptor.SectorsPerFAT = RevByte( buf[0x16],buf[0x17] );
	bdptor.SectorsPerTrack = RevByte(buf[0x18],buf[0x19]);
	bdptor.Heads = RevByte(buf[0x1a],buf[0x1b]);
	bdptor.HiddenSectors = RevByte(buf[0x1c],buf[0x1d]);


	printf("Oem_name \t\t%s\n"
		"BytesPerSector \t\t%d\n"
		"SectorsPerCluster \t%d\n"
		"ReservedSector \t\t%d\n"
		"FATs \t\t\t%d\n"
		"RootDirEntries \t\t%d\n"
		"LogicSectors \t\t%d\n"
		"MedioType \t\t%d\n"
		"SectorPerFAT \t\t%d\n"
		"SectorPerTrack \t\t%d\n"
		"Heads \t\t\t%d\n"
		"HiddenSectors \t\t%d\n",
		bdptor.Oem_name,
		bdptor.BytesPerSector,
		bdptor.SectorsPerCluster,
		bdptor.ReservedSectors,
		bdptor.FATs,
		bdptor.RootDirEntries,
		bdptor.LogicSectors,
		bdptor.MediaType,
		bdptor.SectorsPerFAT,
		bdptor.SectorsPerTrack,
		bdptor.Heads,
		bdptor.HiddenSectors);
}

/*����*/
void findDate(unsigned short *year,
			  unsigned short *month,
			  unsigned short *day,
			  unsigned char info[2])
{
	int date;
	date = RevByte(info[0],info[1]);

	*year = ((date & MASK_YEAR)>> 9 )+1980;
	*month = ((date & MASK_MONTH)>> 5);
	*day = (date & MASK_DAY);
}

/*ʱ��*/
void findTime(unsigned short *hour,
			  unsigned short *min,
			  unsigned short *sec,
			  unsigned char info[2])
{
	int time;
	time = RevByte(info[0],info[1]);

	*hour = ((time & MASK_HOUR )>>11);
	*min = (time & MASK_MIN)>> 5;
	*sec = (time & MASK_SEC) * 2;
}

/*
*�ļ�����ʽ�������ڱȽ�
*/
void FileNameFormat(unsigned char *name)
{
	unsigned char *p = name;
	while(*p!='\0')
		p++;
	p--;
	while(*p==' ')
		p--;
	p++;
	*p = '\0';
}

/*������entry�����ͣ�struct Entry*
*����ֵ���ɹ����򷵻�ƫ��ֵ��ʧ�ܣ����ظ�ֵ
*���ܣ��Ӹ�Ŀ¼���ļ����еõ��ļ�����
*/
int GetEntry(struct Entry *pentry)
{
	int ret,i;
	int count = 0;
	unsigned char buf[DIR_ENTRY_SIZE], info[2];

	/*��һ��Ŀ¼�����32�ֽ�*/
	if( (ret = read(fd,buf,DIR_ENTRY_SIZE))<0)
		perror("read entry failed");
	count += ret;

	if(buf[0]==0xe5 || buf[0]== 0x00)
		return -1*count;
	else
	{
		/*���ļ��������Ե�*/
		while (buf[11]== 0x0f) 
		{
			if((ret = read(fd,buf,DIR_ENTRY_SIZE))<0)
				perror("read root dir failed");
			count += ret;
		}

		/*������ʽ���������β��'\0'*/
		for (i=0 ;i<=10;i++)
			pentry->short_name[i] = buf[i];
		pentry->short_name[i] = '\0';

		FileNameFormat(pentry->short_name); 



		info[0]=buf[22];
		info[1]=buf[23];
		findTime(&(pentry->hour),&(pentry->min),&(pentry->sec),info);  

		info[0]=buf[24];
		info[1]=buf[25];
		findDate(&(pentry->year),&(pentry->month),&(pentry->day),info);

		pentry->FirstCluster = RevByte(buf[26],buf[27]);
		pentry->size = RevWord(buf[28],buf[29],buf[30],buf[31]);

		pentry->readonly = (buf[11] & ATTR_READONLY) ?1:0;
		pentry->hidden = (buf[11] & ATTR_HIDDEN) ?1:0;
		pentry->system = (buf[11] & ATTR_SYSTEM) ?1:0;
		pentry->vlabel = (buf[11] & ATTR_VLABEL) ?1:0;
		pentry->subdir = (buf[11] & ATTR_SUBDIR) ?1:0;
		pentry->archive = (buf[11] & ATTR_ARCHIVE) ?1:0;

		return count;
	}
}

/*
*���ܣ���ʾ��ǰĿ¼������
*����ֵ��1���ɹ���-1��ʧ��
*/
int fd_ls()
{
	int ret, offset,cluster_addr;
	struct Entry entry;
	unsigned short seed, next;
	unsigned char buf[DIR_ENTRY_SIZE];
	if( (ret = read(fd,buf,DIR_ENTRY_SIZE))<0)
		perror("read entry failed");
	if(curdir==NULL)
		printf("Root_dir\n");
	else
		printf("%s_dir\n",curdir->short_name);
	printf("\tname\tdate\t\t time\t\tcluster\tsize\t\tattr\n");

	if(curdir==NULL)  /*��ʾ��Ŀ¼��*/
	{
		/*��fd��λ����Ŀ¼������ʼ��ַ*/
		if((ret= lseek(fd,ROOTDIR_OFFSET,SEEK_SET))<0)
			perror("lseek ROOTDIR_OFFSET failed");

		offset = ROOTDIR_OFFSET;

		/*�Ӹ�Ŀ¼����ʼ������ֱ����������ʼ��ַ*/
		while(offset < (DATA_OFFSET))
		{
			ret = GetEntry(&entry);

			offset += abs(ret);
			if(ret > 0)
			{
				printf("%12s\t"
					"%d:%d:%d\t"
					"%d:%d:%d   \t"
					"%d\t"
					"%d\t\t"
					"%s\n",
					entry.short_name,
					entry.year,entry.month,entry.day,
					entry.hour,entry.min,entry.sec,
					entry.FirstCluster,
					entry.size,
					(entry.subdir) ? "dir":"file");
			}
		}
	}

	else /*��ʾ��Ŀ¼*/
	{
		cluster_addr = DATA_OFFSET + (curdir->FirstCluster-2) * CLUSTER_SIZE ;
		if((ret = lseek(fd,cluster_addr,SEEK_SET))<0)
			perror("lseek cluster_addr failed");

		seed = curdir->FirstCluster;
		next = curdir->FirstCluster;
		
		offset = cluster_addr;

		/*ֻ��һ�ص�����*/
		A:while(offset<cluster_addr +CLUSTER_SIZE)
		{
			ret = GetEntry(&entry);
			offset += abs(ret);
			if(ret > 0)
			{
				printf("%12s\t"
					"%d:%d:%d\t"
					"%d:%d:%d   \t"
					"%d\t"
					"%d\t\t"
					"%s\n",
					entry.short_name,
					entry.year,entry.month,entry.day,
					entry.hour,entry.min,entry.sec,
					entry.FirstCluster,
					entry.size,
					(entry.subdir) ? "dir":"file");
			}
		}
		next = GetFatCluster(seed);
		if(next != 0xffff){
			seed = next;
			cluster_addr = DATA_OFFSET + (seed-2) * CLUSTER_SIZE ;
			//printf("hello\n");
			goto A;
		}
	}
	return 0;
} 


/*
*������entryname ���ͣ�char
��pentry    ���ͣ�struct Entry*
��mode      ���ͣ�int��mode=1��ΪĿ¼���mode=0��Ϊ�ļ�
*����ֵ��ƫ��ֵ����0����ɹ���-1����ʧ��
*���ܣ�������ǰĿ¼�������ļ���Ŀ¼��
*/
int ScanEntry (char *entryname,struct Entry *pentry,int mode)
{
	int ret,offset,i;
	int cluster_addr;
	unsigned short seed;
	char uppername[80];
	for(i=0;i< strlen(entryname);i++)
		uppername[i]= toupper(entryname[i]);
	uppername[i]= '\0';
	/*ɨ���Ŀ¼*/
	if(curdir ==NULL)  
	{
		if((ret = lseek(fd,ROOTDIR_OFFSET,SEEK_SET))<0)
			perror ("lseek ROOTDIR_OFFSET failed");
		offset = ROOTDIR_OFFSET;


		while(offset<DATA_OFFSET)
		{
			ret = GetEntry(pentry);
			offset +=abs(ret);

			if(pentry->subdir == mode &&!strcmp((char*)pentry->short_name,uppername))

				return offset;

		}
		return -1;
	}

	/*ɨ����Ŀ¼*/
	else  
	{
		seed=curdir->FirstCluster;
		do{
			cluster_addr = DATA_OFFSET + (seed -2)*CLUSTER_SIZE;
			if((ret = lseek(fd,cluster_addr,SEEK_SET))<0)
				perror("lseek cluster_addr failed");
			offset= cluster_addr;

			while(offset<cluster_addr + CLUSTER_SIZE)
			{
				ret= GetEntry(pentry);
				offset += abs(ret);
				if(pentry->subdir == mode &&!strcmp((char*)pentry->short_name,uppername))
					return offset;
			}
			seed = GetFatCluster(seed);
		}while(seed!=0xffff);
		return -1;
	}
}

int fd_cd1(char *dir)
{
	struct Entry *pentry;
	int ret;

	if(strcmp(dir,".")==0)
	{
		return 1;
	}
	if(strcmp(dir,"..")==0 && curdir==NULL)
		return 1;
	/*������һ��Ŀ¼*/
	if(strcmp(dir,"..")==0 && curdir!=NULL)
	{
		free(curdir);
		curdir = fatherdir[dirno];
		fatherdir[dirno]=NULL;
		dirno--; 
		return 1;
	}
	pentry = (struct Entry*)malloc(sizeof(struct Entry));

	ret = ScanEntry(dir,pentry,1);
	if(ret < 0)
	{
		free(pentry);
		return -1;
	}
	dirno ++;
	fatherdir[dirno] = curdir;
	curdir = pentry;
	return 1;
}

/*
*������dir�����ͣ�char
*����ֵ��1���ɹ���-1��ʧ��
*���ܣ��ı�Ŀ¼����Ŀ¼����Ŀ¼
*/
int fd_cd(char *path){
	// struct Entry* fatherdir_copy[10];
	// int dirno_copy = dirno;
	// struct Entry *curdir_copy = curdir;
	// for(k=1;fatherdir[k]!=NULL;k++){
 // 		fatherdir_copy[k] = fatherdir[k];
 // 	}
	char curpath_copy[200];
	get_curdir_path(curpath_copy);
 	int i=0,j,k;
	int ret;

	if(path[0]=='/'){
		i++;
		while(curdir!=NULL){
			ret = fd_cd1("..");
			if(ret<0){
				printf("wrong!!\n");

				return -1;
			}
		}
	}
	char dir[20];
	while(path[i]!='\0'){
		for(j=0;path[i]!='/' && path[i]!='\0';i++,j++){
			dir[j]=path[i];
		}
		if(path[i]=='/'){
			i++;
		}
		dir[j]='\0';
		ret = fd_cd1(dir);
		if(ret<0){
			printf("no this direcotry\n");
			// dirno = dirno_copy;
			// curdir = curdir_copy;
			// for(k=1;fatherdir_copy[k]!=NULL;k++){
			//  	fatherdir[k] = fatherdir_copy[k];
 		// 	}
			fd_cd(curpath_copy);

			return -1;
		}
	}

	return 1;

}

/*
*������prev�����ͣ�unsigned char
*����ֵ����һ��
*��fat���л����һ�ص�λ��
*/
unsigned short GetFatCluster(unsigned short prev)
{
	unsigned short next;
	int index;

	index = prev * 2;
	next = RevByte(fatbuf[index],fatbuf[index+1]);

	return next;
}

/*
*������cluster�����ͣ�unsigned short
*����ֵ��void
*���ܣ����fat���еĴ���Ϣ
*/
void ClearFatCluster(unsigned short cluster)
{
	int index;
	index = cluster * 2;

	fatbuf[index]=0x00;
	fatbuf[index+1]=0x00;

}


/*
*���ı��fat��ֵд��fat��
*/
int WriteFat()
{
	if(lseek(fd,FAT_ONE_OFFSET,SEEK_SET)<0)
	{
		perror("lseek failed");
		return -1;
	}
	if(write(fd,fatbuf,512*256)<0)
	{
		perror("read failed");
		return -1;
	}
	if(lseek(fd,FAT_TWO_OFFSET,SEEK_SET)<0)
	{
		perror("lseek failed");
		return -1;
	}
	if((write(fd,fatbuf,512*256))<0)
	{
		perror("read failed");
		return -1;
	}
	return 1;
}

/*
*��fat�����Ϣ������fatbuf[]��
*/
int ReadFat()
{
	if(lseek(fd,FAT_ONE_OFFSET,SEEK_SET)<0)
	{
		perror("lseek failed");
		return -1;
	}
	if(read(fd,fatbuf,512*256)<0)
	{
		perror("read failed");
		return -1;
	}
	return 1;
}


/*
*������filename�����ͣ�char
*����ֵ��1���ɹ���-1��ʧ��
*����;ɾ����ǰĿ¼�µ��ļ�
*/
int fd_df(char *filename)
{
	struct Entry *pentry;
	int ret;
	unsigned char c;
	unsigned short seed,next;

	pentry = (struct Entry*)malloc(sizeof(struct Entry));

	/*ɨ�赱ǰĿ¼�����ļ�*/
	ret = ScanEntry(filename,pentry,0);
	if(ret<0)
	{
		printf("no such file\n");
		free(pentry);
		return -1;
	}

	/*���fat����*/
	seed = pentry->FirstCluster;
	while((next = GetFatCluster(seed))!=0xffff)
	{
		ClearFatCluster(seed);
		seed = next;
	}

	ClearFatCluster(seed);

	/*���Ŀ¼����*/
	c=0xe5;


	if(lseek(fd,ret-0x20,SEEK_SET)<0)//????
		perror("lseek fd_df failed");
	if(write(fd,&c,1)<0)
		perror("write failed");  


	/*if(lseek(fd,ret-0x40,SEEK_SET)<0)//???
		perror("lseek fd_df failed");
	if(write(fd,&c,1)<0)
		perror("write failed");*/

	free(pentry);
	if(WriteFat()<0)
		exit(1);
	return 1;
}

/*
*������filename�����ͣ�char
*����ֵ��1���ɹ���-1��ʧ��
*����;ɾ����ǰĿ¼�µ��ļ� mode=1 -r
*/
int fd_ddir(char *dirname)
{
	struct Entry *pentry;
	int ret,this_addr;
	unsigned char c;
	unsigned short seed,next;

	pentry = (struct Entry*)malloc(sizeof(struct Entry));

	/*ɨ�赱ǰĿ¼����dir*/
	ret = ScanEntry(dirname,pentry,1);
	if(ret<0)
	{
		printf("no such dir\n");
		free(pentry);
		return -1;
	}
	/*���fat����*/
	seed = pentry->FirstCluster;
	if(lseek(fd,(seed-2)*CLUSTER_SIZE+DATA_OFFSET,SEEK_SET)<0)
		perror("lseek failed");
	if(read(fd,&c,1)<0)
		perror("read failed");
	fd_cd(dirname);
	while((next = GetFatCluster(seed))!=0xffff)
	{
		clearDir(seed);
		ClearFatCluster(seed);
		seed = next;
	}
	clearDir(seed);
	ClearFatCluster(seed);
	fd_cd("..");

	/*���Ŀ¼����*/
	c=0xe5;
	printf("%s deleted\n",pentry->short_name);

	if(lseek(fd,ret-0x20,SEEK_SET)<0)//????
		perror("lseek fd_df failed");
	if(write(fd,&c,1)<0)
		perror("write failed");  


	free(pentry);
	if(WriteFat()<0)
		exit(1);
	return 1;
}

void clearDir(unsigned short cluster){
	int ret,offset,tmp;
	struct Entry entry;
	char c;
	tmp=offset=(cluster-2)*CLUSTER_SIZE+DATA_OFFSET;
	if((ret= lseek(fd,offset,SEEK_SET))<0)
		perror("lseek failed");

		/*ֻ��һ�ص�����*/
		while(offset< tmp+CLUSTER_SIZE)
		{
			ret = GetEntry(&entry);
			offset += abs(ret);
			if(ret > 0){
				if(delete_dir_mode==0){
					printf("This directory is not empty, are you sure you want to delete it?(y/n)\n");
					scanf(" %c",&c);
					if(c=='n')
						return;
					else
						delete_dir_mode=1;
				}
				if(entry.subdir==1)
					fd_ddir(entry.short_name);
				else{
					fd_df(entry.short_name);
					printf("%s deleted\n",entry.short_name);
				}
				if(lseek(fd,offset,SEEK_SET)<0)
					perror("lseek failed");
			}
		}
}


/*
*������filename�����ͣ�char�������ļ�������
size��    ���ͣ�int���ļ��Ĵ�С
*����ֵ��1���ɹ���-1��ʧ��
*���ܣ��ڵ�ǰĿ¼�´����ļ�
*/
int fd_cf(char *filename,char *contain)
{

	struct Entry *pentry;
	int ret,i=0,cluster_addr,offset,this_addr;
	unsigned short cluster,clusterno[100],seed;
	unsigned char c[DIR_ENTRY_SIZE];
	int index,clustersize;
	unsigned char buf[DIR_ENTRY_SIZE];
	time_t timep;
	struct tm *p;
	int t1,t2;
	int size = strlen(contain);
	pentry = (struct Entry*)malloc(sizeof(struct Entry));


	clustersize = (size / (CLUSTER_SIZE));

	if(size % (CLUSTER_SIZE) != 0)
		clustersize ++;

	//ɨ���Ŀ¼���Ƿ��Ѵ��ڸ��ļ���
	ret = ScanEntry(filename,pentry,0);
	if (ret<0)
	{
		/*��ѯfat���ҵ��հ״أ�������clusterno[]��*/
		for(cluster=2;cluster<1000;cluster++)
		{
			index = cluster *2;//fat 16
			if(fatbuf[index]==0x00&&fatbuf[index+1]==0x00)
			{
				clusterno[i] = cluster;

				i++;
				if(i==clustersize)
					break;

			}

		}

		/*��fat����д����һ����Ϣ*/
		for(i=0;i<clustersize-1;i++)
		{
			index = clusterno[i]*2;

			fatbuf[index] = (clusterno[i+1] &  0x00ff);
			fatbuf[index+1] = ((clusterno[i+1] & 0xff00)>>8);


		}
		/*���һ��д��0xffff*/
		index = clusterno[i]*2;
		fatbuf[index] = 0xff;
		fatbuf[index+1] = 0xff;

		for(i=0;i<clustersize;i++){
			cluster_addr = DATA_OFFSET + (clusterno[i]-2) * CLUSTER_SIZE ;
			if ((ret=lseek(fd,cluster_addr,SEEK_SET))<0)
				perror("lseek ROOTDIR_OFFSET failed");
			if ((ret= write(fd,contain,CLUSTER_SIZE))<0)
				perror("write entry failed");
			contain +=CLUSTER_SIZE;
		}

		if(curdir==NULL)  /*����Ŀ¼��д�ļ�*/
		{ 

			if((ret= lseek(fd,ROOTDIR_OFFSET,SEEK_SET))<0)
				perror("lseek ROOTDIR_OFFSET failed");
			offset = ROOTDIR_OFFSET;
			while(offset < DATA_OFFSET)
			{
				if((ret = read(fd,buf,DIR_ENTRY_SIZE))<0)
					perror("read entry failed");

				offset += abs(ret);

				if(buf[0]!=0xe5&&buf[0]!=0x00)
				{
					while(buf[11] == 0x0f)
					{
						if((ret = read(fd,buf,DIR_ENTRY_SIZE))<0)
							perror("read root dir failed");
						offset +=abs(ret);
					}
				}


				/*�ҳ���Ŀ¼�����ɾ����Ŀ¼��*/ 
				else
				{       
					offset = offset-abs(ret);     
					for(i=0;i<=strlen(filename);i++)
					{
						c[i]=toupper(filename[i]);
					}			
					for(;i<=10;i++)
						c[i]=' ';

					c[11] = 0x01;

					//write time
					time(&timep);
					p=localtime(&timep);
					t1= p->tm_hour<<11 | p->tm_min<<5 | p->tm_sec;
					c[23]=t1>>8;
					c[22]=t1;
					t2= (1900+p->tm_year-1980)<<9 | (1+p->tm_mon)<<5 |p->tm_mday;
					c[25]=t2>>8;
					c[24]=t2;

					/*д��һ�ص�ֵ*/
					c[26] = (clusterno[0] &  0x00ff);
					c[27] = ((clusterno[0] & 0xff00)>>8);

					
					/*д�ļ��Ĵ�С*/
					c[28] = (size &  0x000000ff);
					c[29] = ((size & 0x0000ff00)>>8);
					c[30] = ((size& 0x00ff0000)>>16);
					c[31] = ((size& 0xff000000)>>24);

					if(lseek(fd,offset,SEEK_SET)<0)
						perror("lseek fd_cf failed");
					if(write(fd,&c,DIR_ENTRY_SIZE)<0)
						perror("write failed");
					
					/*for(i=0;i<clustersize;i++)
					{
					this_addr=(clusterno[i]-2)*CLUSTER_SIZE + DATA_OFFSET;
					if(lseek(fd,this_addr,SEEK_SET)<0)
						perror("lseek fd_cf failed");
					if(write(fd,tmp,2048)<0)
						perror("write failed");
					}*/


					free(pentry);
					if(WriteFat()<0)
						exit(1);

					return 1;
				}

			}
		}
		else 
		{
			seed = curdir->FirstCluster;
			do{
				cluster_addr = (seed -2 )*CLUSTER_SIZE + DATA_OFFSET;
				if((ret= lseek(fd,cluster_addr,SEEK_SET))<0)
					perror("lseek cluster_addr failed");
				offset = cluster_addr;
				while(offset < cluster_addr + CLUSTER_SIZE)
				{
					if((ret = read(fd,buf,DIR_ENTRY_SIZE))<0)
						perror("read entry failed");

					offset += abs(ret);

					if(buf[0]!=0xe5&&buf[0]!=0x00)
					{
						while(buf[11] == 0x0f)
						{
							if((ret = read(fd,buf,DIR_ENTRY_SIZE))<0)
								perror("read root dir failed");
							offset +=abs(ret);
						}
					}
					else
					{ 
						offset = offset - abs(ret);      
						for(i=0;i<=strlen(filename);i++)
						{
							c[i]=toupper(filename[i]);
						}
						for(;i<=10;i++)
							c[i]=' ';

						c[11] = 0x01;

						//write time
						time(&timep);
						p=localtime(&timep);
						t1= p->tm_hour<<11 | p->tm_min<<5 | p->tm_sec;
						c[23]=t1>>8;
						c[22]=t1;
						t2= (1900+p->tm_year-1980)<<9 | (1+p->tm_mon)<<5 |p->tm_mday;
						c[25]=t2>>8;
						c[24]=t2;

						c[26] = (clusterno[0] &  0x00ff);
						c[27] = ((clusterno[0] & 0xff00)>>8);

						c[28] = (size &  0x000000ff);
						c[29] = ((size & 0x0000ff00)>>8);
						c[30] = ((size& 0x00ff0000)>>16);
						c[31] = ((size& 0xff000000)>>24);

						if(lseek(fd,offset,SEEK_SET)<0)
							perror("lseek fd_cf failed");
						if(write(fd,&c,DIR_ENTRY_SIZE)<0)
							perror("write failed");


						free(pentry);
						if(WriteFat()<0)
							exit(1);

						return 1;
					}

				}
				seed = GetFatCluster(seed);
			}while(seed!=0xffff);
		}
	}
	else
	{
		printf("This filename is exist\n");
		free(pentry);
		return -1;
	}
	return 1;

}

/*
*������dirname�����ͣ�char�������ļ�������
size��    ���ͣ�int���ļ��Ĵ�С
*����ֵ��1���ɹ���-1��ʧ��
*���ܣ��ڵ�ǰĿ¼�´����ļ�
*/
int fd_cdir(char *dirname)
{
	struct Entry *pentry;
	int ret,i=0,cluster_addr,offset,this_addr;
	unsigned short cluster,clusterno[100],seed;
	unsigned char c[DIR_ENTRY_SIZE],tmp[32]={0};
	int index,clustersize,size=2048*4;
	unsigned char buf[DIR_ENTRY_SIZE];
	time_t timep;
	struct tm *p;
	int t1,t2;
	pentry = (struct Entry*)malloc(sizeof(struct Entry));


	clustersize = (size / (CLUSTER_SIZE));

	if(size % (CLUSTER_SIZE) != 0)
		clustersize ++;

	//ɨ���Ŀ¼���Ƿ��Ѵ��ڸ�dir
	ret = ScanEntry(dirname,pentry,1);
	if (ret<0)
	{
		/*��ѯfat���ҵ��հ״أ�������clusterno[]��*/
		for(cluster=2;cluster<1000;cluster++)
		{
			index = cluster *2;//fat 16
			if(fatbuf[index]==0x00&&fatbuf[index+1]==0x00)
			{
				clusterno[i] = cluster;

				i++;
				if(i==clustersize)
					break;

			}

		}

		/*��fat����д����һ����Ϣ*/
		for(i=0;i<clustersize-1;i++)
		{
			index = clusterno[i]*2;

			fatbuf[index] = (clusterno[i+1] &  0x00ff);
			fatbuf[index+1] = ((clusterno[i+1] & 0xff00)>>8);


		}
		/*���һ��д��0xffff*/
		index = clusterno[i]*2;
		fatbuf[index] = 0xff;
		fatbuf[index+1] = 0xff;

		if(curdir==NULL)  /*����Ŀ¼��д�ļ�*/
		{ 

			if((ret= lseek(fd,ROOTDIR_OFFSET,SEEK_SET))<0)
				perror("lseek ROOTDIR_OFFSET failed");
			offset = ROOTDIR_OFFSET;
			while(offset < DATA_OFFSET)
			{
				if((ret = read(fd,buf,DIR_ENTRY_SIZE))<0)
					perror("read entry failed");

				offset += abs(ret);

				if(buf[0]!=0xe5&&buf[0]!=0x00)
				{
					while(buf[11] == 0x0f)
					{
						if((ret = read(fd,buf,DIR_ENTRY_SIZE))<0)
							perror("read root dir failed");
						offset +=abs(ret);
					}
				}


				/*�ҳ���Ŀ¼�����ɾ����Ŀ¼��*/ 
				else
				{       
					offset = offset-abs(ret);     
					for(i=0;i<=strlen(dirname);i++)
					{
						c[i]=toupper(dirname[i]);
					}			
					for(;i<=10;i++)
						c[i]=' ';

					c[11] = 0x10;//is a dir

					//write time
					time(&timep);
					p=localtime(&timep);
					t1= p->tm_hour<<11 | p->tm_min<<5 | p->tm_sec;
					c[23]=t1>>8;
					c[22]=t1;
					t2= (1900+p->tm_year-1980)<<9 | (1+p->tm_mon)<<5 | p->tm_mday;
					c[25]=t2>>8;
					c[24]=t2;

					/*д��һ�ص�ֵ*/
					c[26] = (clusterno[0] &  0x00ff);
					c[27] = ((clusterno[0] & 0xff00)>>8);

					/*дdir�Ĵ�С=8192*/
					c[28] = (size &  0x000000ff);
					c[29] = ((size & 0x0000ff00)>>8);
					c[30] = ((size& 0x00ff0000)>>16);
					c[31] = ((size& 0xff000000)>>24);

					if(lseek(fd,offset,SEEK_SET)<0)
						perror("lseek fd_cf failed");
					if(write(fd,&c,DIR_ENTRY_SIZE)<0)
						perror("write failed");

					this_addr=(cluster-2)*CLUSTER_SIZE + DATA_OFFSET;
					if(lseek(fd,this_addr,SEEK_SET)<0)
						perror("lseek fd_cf failed");
					for(i=0;i<size/32;i++)
						if(write(fd,tmp,32)<0)
							perror("write failed");


					free(pentry);
					if(WriteFat()<0)
						exit(1);

					return 1;
				}

			}
		}
		else 
		{
			seed = curdir->FirstCluster;
			do{
				cluster_addr = (seed -2 )*CLUSTER_SIZE + DATA_OFFSET;
				if((ret= lseek(fd,cluster_addr,SEEK_SET))<0)
					perror("lseek cluster_addr failed");
				offset = cluster_addr;
				while(offset < cluster_addr + CLUSTER_SIZE)
				{
					if((ret = read(fd,buf,DIR_ENTRY_SIZE))<0)
						perror("read entry failed");

					offset += abs(ret);

					if(buf[0]!=0xe5&&buf[0]!=0x00)
					{
						while(buf[11] == 0x0f)
						{
							if((ret = read(fd,buf,DIR_ENTRY_SIZE))<0)
								perror("read root dir failed");
							offset +=abs(ret);
						}
					}
					else
					{ 
						offset = offset - abs(ret);      
						for(i=0;i<=strlen(dirname);i++)
						{
							c[i]=toupper(dirname[i]);
						}
						for(;i<=10;i++)
							c[i]=' ';

						c[11] = 0x10;//is a dir

						//write time
						time(&timep);
						p=localtime(&timep);
						t1= p->tm_hour<<11 | p->tm_min<<5 | p->tm_sec;
						c[23]=t1>>8;
						c[22]=t1;
						t2= (1900+p->tm_year-1980)<<9 | (1+p->tm_mon)<<5 |p->tm_mday;
						c[25]=t2>>8;
						c[24]=t2;

						/*д��һ�ص�ֵ*/
						c[26] = (clusterno[0] &  0x00ff);
						c[27] = ((clusterno[0] & 0xff00)>>8);

						/*дdir�Ĵ�С=8192*/
						c[28] = (size &  0x000000ff);
						c[29] = ((size & 0x0000ff00)>>8);
						c[30] = ((size& 0x00ff0000)>>16);
						c[31] = ((size& 0xff000000)>>24);

						if(lseek(fd,offset,SEEK_SET)<0)
							perror("lseek fd_cf failed");
						if(write(fd,&c,DIR_ENTRY_SIZE)<0)
							perror("write failed");

						this_addr=(cluster-2)*CLUSTER_SIZE + DATA_OFFSET;
						if(lseek(fd,this_addr,SEEK_SET)<0)
							perror("lseek fd_cf failed");
						for(i=0;i<size/32;i++)
							if(write(fd,tmp,32)<0)
								perror("write failed");


						free(pentry);
						if(WriteFat()<0)
							exit(1);

						return 1;
					}

				}
				seed = GetFatCluster(seed);
			}while(seed!=0xffff);
		}
	}
	else
	{
		printf("This dirname is exist\n");
		free(pentry);
		return -1;
	}
	return 1;

}

void get_curdir_path(char* path){
	int i;
	int j=0;
	struct Entry* dir;
	

	for(i=1;fatherdir[i]!=NULL;i++){
		dir = fatherdir[i];
		path[j++]='/';
		strcpy(&path[j],dir->short_name);
		j+=strlen(dir->short_name);
	}
	if(curdir==NULL){
		path[j++]='/';
	}
	else{
		path[j++]='/';
		strcpy(&path[j],curdir->short_name);
		j+=strlen(curdir->short_name);
	}
	path[j]=0;
}

int fd_pwd(){
	char path[200];
	get_curdir_path(path);
	printf("%s\n",path);
	return 1;
}

void do_usage()
{
	printf("please input a command, including followings:\n\tls\t\t\tlist all files\n\tcd <dir>\t\tchange direcotry\n\tcf <filename> <size>\tcreate a file\n\tdf <file>\t\tdelete a file\n\texit\t\t\texit this system\n");
}


int main()
{
	char input[10];
	int size=0;
	char name[12];
	char inputcontain[CLUSTER_SIZE]={'\0'};
	if((fd = open(DEVNAME,O_RDWR))<0)
		perror("open failed");
	ScanBootSector();
	if(ReadFat()<0)
		exit(1);
	do_usage();
	while (1)
	{
		printf(">");
		scanf("%s",input);

		if (strcmp(input, "exit") == 0)
			break;
		else if (strcmp(input, "ls") == 0)
			fd_ls();
		else if(strcmp(input, "cd") == 0)
		{
			scanf("%s", name);
			fd_cd(name);
		}
		else if(strcmp(input, "df") == 0)
		{
			scanf("%s", name);
			fd_df(name);
		}
		else if(strcmp(input, "cf") == 0)
		{
			scanf("%s ", name);
			gets(inputcontain);
			fd_cf(name,inputcontain);
			memset(inputcontain,0,CLUSTER_SIZE);
		}
		else if(strcmp(input, "mkdir") == 0){
			scanf("%s",name);
			fd_cdir(name);
		}
		else if(strcmp(input, "rm") == 0){
			scanf("%s",name);
			if(strcmp(name,"-r")==0){
				scanf("%s",name);
				delete_dir_mode=1;
				fd_ddir(name);
			}
			else{
				delete_dir_mode=0;
				fd_ddir(name);
			}
		}
		else
			do_usage();
	}	

	return 0;
}



