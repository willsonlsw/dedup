#include<stdio.h>
#include<string.h>
#include<time.h>
#include<unistd.h>
#include<stdlib.h>
#include<semaphore.h>
#include<pthread.h>
#include "dedup.h"
#include "rabin-fingerprint.h"
#include "cpu-sha1.h"
#include "hash-search.h"

int sign_cal(char *str){
	unsigned int checksum=0;
	int len=strlen(str);
	for(int i=0;i<len;i++){
		if(i<4) checksum=checksum*PRIME_BASE+str[i];
		else checksum=(checksum-str[i-4]*SOUT_BASE)*PRIME_BASE+str[i];
	}
	return checksum & MOD_SUM;
}

sem_t sep_input_in,sep_input_out,sep_rabin_in,sep_rabin_out;
sem_t sep_sha1_in,sep_sha1_out,sep_search_in,sep_search_out;
sem_t finish;

clock_t input_time=0,rabin_time=0,sha1_time=0,hash_time=0;
clock_t insta,inend,rasta,raend,shsta,shend,sesta,seend;
int sign;
struct input_buf *buffer_tmp,*buffer_rabin,*buffer_sha1,*buffer_search;
unsigned int *offset_rabin,*offset_sha1,*offset_search;
unsigned int num,num_sha1,num_search,total_num,total_file_len;
unsigned char *output_sha1;
FILE *inf,*outf;
int search_times,input_times;

void *thread_rabin(void *arg){
	while(1){
		sem_wait(&sep_rabin_in);
		buffer_rabin=buffer_tmp;
		offset_rabin=new unsigned int[200000];
		sem_post(&sep_input_out);
		rasta=clock();
		rabin_finger(2048,8192,sign,buffer_rabin->buf,offset_rabin,num,buffer_rabin->fill_buf_len);
		raend=clock();
		rabin_time+=(raend-rasta);
		printf("chunk num: %d\n",num);
		total_num+=num;
		sem_post(&sep_sha1_in);
		sem_wait(&sep_rabin_out);
	}
}

void *thread_sha1(void *arg){
	while(1){
		sem_wait(&sep_sha1_in);
		buffer_sha1=buffer_rabin;
		offset_sha1=offset_rabin;
		num_sha1=num;
		output_sha1=new unsigned char[FINGERPRINT_LEN*num_sha1];
		sem_post(&sep_rabin_out);
		shsta=clock();
		CPU_sha1(buffer_sha1->buf,output_sha1,offset_sha1,num_sha1,buffer_sha1->fill_buf_len);
		shend=clock();
		sha1_time+=(shend-shsta);
		sem_post(&sep_search_in);
		sem_wait(&sep_sha1_out);
	}
}

void *thread_search(void *arg){
	while(1){
		sem_wait(&sep_search_in);
		sha1_type *sha1_type_val=(sha1_type*)output_sha1;
		buffer_search=buffer_sha1;
		offset_search=offset_sha1;
		num_search=num_sha1;
		sem_post(&sep_sha1_out);
		sesta=clock();
		hash_search(outf,buffer_search,offset_search,sha1_type_val,num_search);
		seend=clock();
		hash_time+=(seend-sesta);
		delete[] sha1_type_val;
		delete[] offset_search; 
		delete[] buffer_search->buf;
		search_times++;
		if(search_times==input_times) sem_post(&finish);
	}
}

int main(int argc,char *argv[]){
	sign=1;
	if(argc==3 && strcmp(argv[2],"")!=0) sign=sign_cal(argv[2]);
	 
	total_num=0,total_file_len=0;
	inf=fopen(argv[1],"r");
	outf=fopen("chunk.ded","w");

	sem_init(&sep_input_in,0,1);
	sem_init(&sep_input_out,0,0);
	sem_init(&sep_rabin_in,0,0);
	sem_init(&sep_rabin_out,0,0);
	sem_init(&sep_sha1_in,0,0);
	sem_init(&sep_sha1_out,0,0);
	sem_init(&sep_search_in,0,0);
	sem_init(&sep_search_out,0,0);
	sem_init(&finish,0,0);
	pthread_t rabinpid,sha1pid,searchpid;
	pthread_create(&rabinpid,NULL,thread_rabin,NULL);
	pthread_create(&sha1pid,NULL,thread_sha1,NULL);
	pthread_create(&searchpid,NULL,thread_search,NULL);
	
	search_times=input_times=0;
	clock_t sta=clock();
	insta=clock();
	struct input_buf *buffer=new struct input_buf;
	while((buffer->fill_buf_len=fread(buffer->buf,1,buffer->buf_size,inf))>0){
		inend=clock();
		input_time+=(inend-insta);
		input_times++;
		
		total_file_len+=buffer->fill_buf_len;
		printf("file len: %d\n",buffer->fill_buf_len);
		buffer_tmp=buffer;
		sem_post(&sep_rabin_in);
		sem_wait(&sep_input_out);
		insta=clock();
		buffer=new struct input_buf;
	}
	sem_wait(&finish);
	clock_t end=clock();
/*
		sta=clock();
		rabin_finger(2048,8192,sign,buffer->buf,offset,num,buffer->fill_buf_len);
		end=clock();
		rabin_time+=(end-sta);
		
		printf("chunk num: %d\n",num);
		printf("file len:%d\n",buffer->fill_buf_len);
		total_num+=num;
		total_file_len+=buffer->fill_buf_len;

		unsigned char *output=new unsigned char[FINGERPRINT_LEN*num];
		sta=clock();
		CPU_sha1(buffer->buf,output,offset,num,buffer->fill_buf_len);
		end=clock();
		sha1_time+=(end-sta);

		sha1_type *sha1_type_val=(sha1_type*)output;
		sta=clock();
		hash_search(outf,buffer,offset,sha1_type_val,num);
		end=clock();
		hash_time+=(end-sta);

		free(output);
		sta=clock();
*/

	printf("total chunk num: %d\n",total_num);
	printf("total file len: %d\n",total_file_len);

	printf("input data time: %ld\n",input_time/1000);
	printf("rabin fingerprint cal time: %ld\n",rabin_time/1000);
	printf("sha1 cal time: %ld\n",sha1_time/1000);
	printf("hash search time: %ld\n",hash_time/1000);
	printf("total time:%ld, %ld\n",(input_time+rabin_time+sha1_time+hash_time)/1000,(end-sta)/1000);

	fclose(inf);
	fclose(outf);
			
	//free(buffer);
	//free(output);
	//free(offset);
	return 0;
}
