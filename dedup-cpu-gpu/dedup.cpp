#include<stdlib.h>
#include<cstdio>
#include<string.h>
#include<time.h>
#include<unistd.h>
#include "gpu-process.h"
#include "dedup.h"
#include "hash-search.h"

int sign_cal(char *str){
	unsigned int checksum=0;
	int len=strlen(str),i;
	for(i=0;i<len;i++){
		if(i<4) checksum=checksum*PRIME_BASE+str[i];
		else checksum=(checksum-str[i-4]*SOUT_BASE)*PRIME_BASE+str[i];
	}
	return checksum & MOD_SUM;
}

clock_t input_time=0,rabin_time=0,sha1_time=0,hash_time=0;
clock_t sta,end;

int main(int argc,char *argv[]){
	int sign=1;
	if(argc==3 && strcmp(argv[2],"")!=0) sign=sign_cal(argv[2]);
	 
	struct input_buf *buffer = new struct input_buf;
	unsigned int *offset=new unsigned int[1000000];
	unsigned int num,total_num=0;
	long long total_file_len=0;
	FILE *inf=fopen(argv[1],"r");
	FILE *outf=fopen("chunk.ded","w");
	
	sta=clock();
	while((buffer->fill_buf_len=fread(buffer->buf,1,buffer->buf_size,inf))>0){
		end=clock();
		input_time+=(end-sta);
		
		sta=clock();
		GPU_rabin(4096,8192,sign,buffer->buf,offset,num,buffer->fill_buf_len);
		end=clock();
		rabin_time+=(end-sta);
		
		printf("chunk num: %d\n",num);
		printf("file len:%d\n",buffer->fill_buf_len);
		total_num+=num;
		total_file_len+=buffer->fill_buf_len;

		unsigned char *output=new unsigned char[FINGERPRINT_LEN*num];
		sta=clock();
		GPU_sha1(buffer->buf,output,offset,num,buffer->fill_buf_len);
		end=clock();
		sha1_time+=(end-sta);

		sha1_type *sha1_type_val=(sha1_type*)output;
		sta=clock();
		hash_search(outf,buffer,offset,sha1_type_val,num);
		end=clock();
		hash_time+=(end-sta);

		delete[] output;
		sta=clock();
	}

	printf("total chunk num: %d\n",total_num);
	printf("total file len: %ld\n",total_file_len);

	printf("input data time: %ld\n",input_time/1000);
	printf("rabin fingerprint cal time: %ld\n",rabin_time/1000);
	printf("sha1 cal time: %ld\n",sha1_time/1000);
	printf("hash search time: %ld\n",hash_time/1000);
/*	
	FILE *outf=fopen("sha1-out.txt","w");
	int k=0;
	unsigned int *sha1_int=(unsigned int*)output;
	for(int i=0;i<num;i++){
		fprintf(outf,"%d:",i);
		for(int j=0;j<5;j++){
			fprintf(outf,"%08x ",sha1_int[k++]);
		}
		fprintf(outf,"\n");
	}

	sha1_type *sha1_type_val=(sha1_type*)output;
	for(int i=0;i<num;i++){
		fprintf(outf,"%d:",i);
		fprintf(outf,"%08x %08x %08x %08x %08x\n",sha1_type_val[i][0],sha1_type_val[i][1],sha1_type_val[i][2],sha1_type_val[i][3],sha1_type_val[i][4]);
	}
*/
	fclose(inf);
	fclose(outf);
			
	free(buffer);
	//free(output);
	delete[] offset;
	return 0;
}
