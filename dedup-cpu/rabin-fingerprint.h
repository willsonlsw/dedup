#define PRIME_BASE 13
#define SOUT_BASE 13*13*13
#define MOD_SUM 0x00001FFF

void rabin_finger(int min_len,int max_len,int sign,unsigned char *input,unsigned int *offset,unsigned int &num,long long len){
	unsigned int checksum=0,finger;
	long long i,last_cut_point;
	bool cut;
	int cut_of_sign=0;
	num=0;
	offset[num++]=0;
	last_cut_point=0;
	cut=0;
	for(i=0;i<len;i++){
		if(cut){
			offset[num++]=i;
			cut=0;
		}
		if(i<4) checksum=checksum*PRIME_BASE+input[i];
		else checksum=(checksum-input[i-4]*SOUT_BASE)*PRIME_BASE+input[i];
		finger=checksum & MOD_SUM;
		if(finger==sign || finger==sign+3){
			if(i-last_cut_point>=min_len){
				cut=1;
				last_cut_point=i;
				cut_of_sign++;	
			}
		}else if(i-last_cut_point>max_len){
			cut=1;
			last_cut_point=i;
		}
	}
	printf("chunk num that cut of sign:%d\n",cut_of_sign);
	offset[num]=len;
}
