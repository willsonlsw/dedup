#include<cstdio>
#include<iostream>
#include<ctime>
#include<cstring>
#include<unistd.h>
#include<sys/types.h>
#include<sys/stat.h>
using namespace std;

int pack_n;
char *p_filename[100];
const char *filename="pack.pk";
char *u_filename;

void file_pack(){
	puts("How many files would you want to pack?");
	scanf("%d",&pack_n);
	for(int i=0;i<pack_n;i++){
		p_filename[i]=new char;
	}
	puts("Please input file names");
	for(int i=0;i<pack_n;i++){
		printf("%d: ",i+1);
		scanf("%s",p_filename[i]);
	}
	
	FILE *fin=NULL;
	FILE *fout=fopen(filename,"w");
	fwrite(&pack_n,sizeof(pack_n),1,fout);
	struct stat st;
	int filename_len; 
	size_t read_len;
	long long file_size;
	char buff[1024]={'\0'};
	for(int i=0;i<pack_n;i++){
		fin=fopen(p_filename[i],"r");
		memset(&st,0,sizeof(st));
		stat(p_filename[i],&st);
		filename_len=strlen(p_filename[i]);
		file_size=(long long)st.st_size;

		fwrite(&filename_len,sizeof(filename_len),1,fout);
		fwrite(p_filename[i],filename_len,1,fout);
		fwrite(&file_size,sizeof(file_size),1,fout);
			
		while((read_len=fread(buff,1,1024,fin))>0){
			fwrite(buff,read_len,1,fout);
		}
		fclose(fin);
	}
	fclose(fout);
}

void file_unpack(){
	puts("Please input unpack-file name");
	u_filename=new char;
	scanf("%s",u_filename);

	size_t read_len,len;
	int filename_len;
	char outfilename[100];
	long long file_size;
	char buff[1024]={'\0'};

	FILE *fin=fopen(u_filename,"r");
	FILE *fout=NULL;
	if((read_len=fread(&pack_n,4,1,fin))==0){
		puts("Read file error!");
		return;
	}else{
		for(int i=0;i<pack_n;i++){
			read_len=fread(&filename_len,4,1,fin);
			memset(outfilename,'\0',sizeof(outfilename));
			read_len=fread(outfilename,filename_len,1,fin);
			read_len=fread(&file_size,sizeof(file_size),1,fin);

			outfilename[0]+=1;
			fout=fopen(outfilename,"w");
			if(file_size<1024) len=file_size;
			else len=1024;
			while(len>0 && (read_len=fread(buff,1,len,fin))>0){
				fwrite(buff,len,1,fout);
				file_size=file_size-len;
				if(file_size<1024) len=file_size;
				else len=1024;
			}
			fclose(fout);
		}
	}
	fclose(fin);
}

void process_choice(){
	puts("pack or unpack?(p/u)");
	char choice;
	cin>>choice;
	if(choice=='p'){
		file_pack();
	}
	else{
		file_unpack();
	}
}

int main(){
	process_choice();
	puts("finish!");
	return 0;
}
