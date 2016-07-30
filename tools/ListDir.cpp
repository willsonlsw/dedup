#include<sys/types.h>
#include<sys/stat.h>
#include<dirent.h>
#include<unistd.h>
#include<stdio.h>
#include<string.h>

int IsDir(const char *path){
	struct stat st;
	stat(path,&st);
	return S_ISDIR(st.st_mode);
}

void ListDir(const char *path){
	DIR *dir;
	struct dirent *ptr;
	char filename[256];
	dir=opendir(path);
	while((ptr=readdir(dir))!=NULL){
		if((strcmp(ptr->d_name,"..")==0)||(strcmp(ptr->d_name,".")==0)) continue;
		sprintf(filename,"%s/%s",path,ptr->d_name);
		printf("%s\n",filename);
		if(IsDir(filename)){
			ListDir(filename);
		}
	}
	closedir(dir);
}

int main(){
	char *dirname=new char;
	scanf("%s",dirname);
	ListDir(dirname);
	return 0;
}

