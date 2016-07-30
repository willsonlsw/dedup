#include<iostream>
#include<stdio.h>
#include<dirent.h>
#include<unistd.h>
#include<string.h>
#include<sys/stat.h>
#include<sys/types.h>
using namespace std;

//#define DEBUG 
#define DIR_HEAD 0
#define DIR_TAIL 1
#define FILE_SIG 2
#define BLOCK_LEN 4096

int IsDir(const char *path){
	struct stat st;
	stat(path,&st);
	return S_ISDIR(st.st_mode);
}

void AddFileIntoPack(const char *filepath,const char *filename,FILE *indf,FILE *datf,long long &data_offset){
	size_t read_len;
	struct stat st;
	long long file_size;
	char buff[BLOCK_LEN+1]={'\0'};
	int filename_len;
	FILE *fin=fopen(filepath,"r");
	stat(filepath,&st);
	filename_len=strlen(filename);
	file_size=(long long)st.st_size;
	
	char file_type=FILE_SIG;
	fwrite(&file_type,sizeof(file_type),1,indf);	
	fwrite(&filename_len,sizeof(filename_len),1,indf);
	fwrite(filename,filename_len,1,indf);
	fwrite(&file_size,sizeof(file_size),1,indf);
	fwrite(&data_offset,sizeof(data_offset),1,indf);
	data_offset+=file_size;
	while((read_len=fread(buff,1,BLOCK_LEN,fin))>0){
		fwrite(buff,read_len,1,datf);
	}
	fclose(fin);
}

void SearchDir(const char *path,FILE *indf,FILE *datf,int dir_id,long long &data_offset){
	int ret;
	DIR *dir;
	struct dirent *ptr;
	char filepath[256];
	int filename_len;
	FILE *fin=NULL;
	char file_type;
	dir=opendir(path);
	while((ptr=readdir(dir))!=NULL){
		if(strcmp(ptr->d_name,"..")==0 || strcmp(ptr->d_name,".")==0)
			continue;
		memset(filepath,'\0',sizeof(filepath));
		sprintf(filepath,"%s/%s",path,ptr->d_name);

#ifdef DEBUG
		printf("%s\n",filepath);
#endif

		if((ret=IsDir(filepath))==1){
			filename_len=strlen(ptr->d_name);	
			dir_id++;
			file_type=DIR_HEAD;
			fwrite(&file_type,sizeof(file_type),1,indf);
			fwrite(&filename_len,sizeof(filename_len),1,indf);
			fwrite(ptr->d_name,filename_len,1,indf);
			fwrite(&dir_id,sizeof(dir_id),1,indf);
			SearchDir(filepath,indf,datf,dir_id,data_offset);
			file_type=DIR_TAIL;
			fwrite(&file_type,sizeof(file_type),1,indf);
			fwrite(&dir_id,sizeof(dir_id),1,indf);
		}else if (ret==0){
			AddFileIntoPack(filepath,ptr->d_name,indf,datf,data_offset);
		}else{
			puts("Run Error!!");
			return;
		}
	}
	closedir(dir);
}

void CreatTar(const char *srcpath){
	int ret;

	char dstname[256];
	strcpy(dstname,srcpath);
	strcat(dstname,".ind");
	FILE *indf=fopen(dstname,"w");
	memset(dstname,'\0',sizeof(dstname));
	strcpy(dstname,srcpath);
	strcat(dstname,".dat");
	FILE *datf=fopen(dstname,"w");

	int dir_id=0;
	long long data_offset=0;

	if((ret=IsDir(srcpath))==0){
		AddFileIntoPack(srcpath,srcpath,indf,datf,data_offset);
		fclose(indf);
		fclose(datf);
		return;
	}else if(ret==-1) puts("No such file or dir!");
		
	char file_type=DIR_HEAD;
	int filename_len=strlen(srcpath);
	fwrite(&file_type,sizeof(file_type),1,indf);
	fwrite(&filename_len,sizeof(filename_len),1,indf);
	fwrite(srcpath,filename_len,1,indf);
	fwrite(&dir_id,sizeof(dir_id),1,indf);
	SearchDir(srcpath,indf,datf,dir_id,data_offset);
	file_type=DIR_TAIL;
	fwrite(&file_type,sizeof(file_type),1,indf);
	fwrite(&dir_id,sizeof(dir_id),1,indf);

	fclose(indf);
	fclose(datf);
}

void ExtraFile(const char *path,FILE *indf,FILE *datf){
	int name_len;
	long long file_size;
	long long file_offset;
	char filename[256],filepath[256];
	int read_len;
	memset(filename,'\0',sizeof(filename));
	memset(filepath,'\0',sizeof(filepath));
	read_len=fread(&name_len,sizeof(name_len),1,indf);
	read_len=fread(filename,name_len,1,indf);
	read_len=fread(&file_size,sizeof(file_size),1,indf);
	read_len=fread(&file_offset,sizeof(file_offset),1,indf);
	if(strcmp(path,"")==0){
		sprintf(filepath,"%s",filename);
	}else{
		sprintf(filepath,"%s/%s",path,filename);
	}
#ifdef DEBUG
	printf("%s\n",filepath);
#endif
	fseek(datf,file_offset,SEEK_SET);
	FILE *outf=fopen(filepath,"w");
	int len;
	char buff[BLOCK_LEN+1]={'\0'};
	if(file_size<BLOCK_LEN) len=file_size;
	else len=BLOCK_LEN;
	while(len>0 && (read_len=fread(buff,1,len,datf))>0){
		fwrite(buff,len,1,outf);
		file_size-=len;
		if(file_size<BLOCK_LEN) len=file_size;
		else len=BLOCK_LEN;
	}
	fclose(outf);
}

void ExtraDir(const char *path,FILE *indf,FILE *datf){
	int read_len;
	char file_type;
	int name_len,dir_id;
	char dirname[256];
	while((read_len=fread(&file_type,sizeof(file_type),1,indf))>0){
		if(file_type==FILE_SIG){
			ExtraFile(path,indf,datf);
		}else if(file_type==DIR_HEAD){
			memset(dirname,'\0',sizeof(dirname));
			read_len=fread(&name_len,sizeof(name_len),1,indf);
			read_len=fread(dirname,name_len,1,indf);
			read_len=fread(&dir_id,sizeof(dir_id),1,indf);
			char dirpath[256];
			memset(dirpath,'\0',sizeof(dirpath));
			if(strcmp(path,"")==0){
				sprintf(dirpath,"%s",dirname);
			}else{
				sprintf(dirpath,"%s/%s",path,dirname);
			}
#ifdef DEBUG
	printf("%s\n",dirpath);
#endif
			if(mkdir(dirpath,0755)==0){
				ExtraDir(dirpath,indf,datf);
			}
		}else if(file_type==DIR_TAIL){
			read_len=fread(&dir_id,sizeof(dir_id),1,indf);
			return;
		}
	}
}

void ExtraTar(const char *indpath, const char *datpath){
	FILE *indf=fopen(indpath,"r");
	FILE *datf=fopen(datpath,"r");
	ExtraDir("",indf,datf);
	fclose(indf);
	fclose(datf);
}

int main(int argc,char *argv[]){
	if(strcmp(argv[1],"p")==0 && argc==3) CreatTar(argv[2]);
	else if(strcmp(argv[1],"e")==0 && argc==4) ExtraTar(argv[2],argv[3]);
	else puts("Input Error");
	puts("Finish!");
	return 0;
}
