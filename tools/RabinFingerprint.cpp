#include<stdio.h>
#include<time.h>
#include<string.h>
#include<stdlib.h>

#define PRIME_BASE 13
#define RED 13*13*13
#define PRIME_MOD 1000000007
#define MOD_SUM 0x0000000000001FFF

void mergeStr(char *str,int n){
	srand(time(NULL));
	for(int i=0;i<n;i++)
		str[i]=rand()%256;//'a'+rand()%26;
}	

void rabin_finger(char *str,int n,int fin[]){
	long long checksum=(long long)str[0];
	fin[0]=checksum & MOD_SUM;
	for(int i=1;i<n;i++){
		if(i<4) checksum=checksum*PRIME_BASE + str[i];
		else checksum=(checksum-str[i-4]*RED)*PRIME_BASE + str[i];
		fin[i]=checksum & MOD_SUM;
	}
}

char str[20000010];

int main(int argc,char *argv[]){
//	char *str=new char;
	int n;
	int sign;
	int *fin;
	if(strcmp(argv[1],"auto")==0){
		puts("Input data len:");
		scanf("%d",&n);
		scanf("%d",&sign);
		fin=new int[n];
		mergeStr(str,n);
		rabin_finger(str,n,fin);
//		printf("%s\n",str);
	}else if(strcmp(argv[1],"in")==0){
		puts("Input str:");
		scanf("%s",str);
		scanf("%d",&sign);
		n=strlen(str);
		fin=new int[n];
		rabin_finger(str,n,fin);
	}
	bool hash[8192];
	memset(hash,0,sizeof(hash));
	int m=0;
	for(int i=0;i<n;i++){
		if(fin[i]==sign) m++;
		hash[fin[i]]=1;
	}
	int k=0;
	for(int i=0;i<8192;i++)
	{
		if(!hash[i]){
			k++;
		}
	}
	printf("%d\n",k);
	printf("%d\n",n/m);	
	return 0;
}
