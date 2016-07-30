#include<cstdio>
//#include<time.h>
#include<string.h>
#include<unistd.h>
#include<stdlib.h>
#include "gpu-process.h"

#define FROM_BIG_ENDIAN(v)                                          \
 ((v & 0xff) << 24) | ((v & 0xff00) << 8) | ((v & 0xff0000) >> 8) |  \
		((v & 0xff000000) >> 24)                              \

#define LEFTROL( v, n)  (v << n) | (v >> (32 - n))
//#define FINGERPRINT_LEN 20
#define MAX_DATA_LEN 8192

unsigned char *gpu_input_data_s,*gpu_output_data_s;
unsigned int *gpu_offset;

__device__ void GPU_sha1_kernel( unsigned char* data_tmp, unsigned int length_tmp,  unsigned int* md)
{

	unsigned int words[80];
	unsigned int H0 = 0x67452301,	H1 = 0xEFCDAB89, H2 = 0x98BADCFE, H3 = 0x10325476, H4 = 0xC3D2E1F0;
	unsigned int a, b, c, d, e, f, k, temp;
	unsigned int i, j;

	unsigned char add_data[8512];
	int kk;
	long long tmp;
	for(kk=0;kk<length_tmp;kk++)
		add_data[kk]=data_tmp[kk];
	if(length_tmp%64<56)
	{
		add_data[kk++]=0x80;
		int t=length_tmp%64+1;
		for(;t<56;t++)
		{
			add_data[kk++]=0x00;
		}
		tmp=length_tmp-(length_tmp%64)+64;
	}else if(length_tmp%64>56)
	{
		add_data[kk++]=0x80;
		int t=length_tmp%64+1;
		for(;t<64;t++)
		{
			add_data[kk++]=0x00;
		}
		for(t=0;t<56;t++)
		{
			add_data[kk++]=0x00;
		}
		tmp=length_tmp-(length_tmp%64)+128;
	}
	add_data[tmp-8]=(tmp & 0xFF00000000000000) >> 56;
	add_data[tmp-7]=(tmp & 0x00FF000000000000) >> 48;
	add_data[tmp-6]=(tmp & 0x0000FF0000000000) >> 40;
	add_data[tmp-5]=(tmp & 0x000000FF00000000) >> 32;
	add_data[tmp-4]=(tmp & 0x00000000FF000000) >> 24;
	add_data[tmp-3]=(tmp & 0x0000000000FF0000) >> 16;
	add_data[tmp-2]=(tmp & 0x000000000000FF00) >> 8;
	add_data[tmp-1]=(tmp & 0x00000000000000FF);

	unsigned int *data=(unsigned int*)add_data;
	unsigned int dataLen=tmp;

	for(j = 0; j < dataLen; j += 64)
	{
		a = H0;
		b = H1;
		c = H2;
		d = H3;
		e = H4;

		for (i=0; i<16; i++)
		{  
			temp = *(( unsigned int*)(data + j/4+i));
			words[i] = FROM_BIG_ENDIAN(temp);
			
			f = (b & c) | ((~b) & d); 
			k = 0x5A827999;
			temp = LEFTROL(a, 5);
			temp += f + e + k + words[i];
			e = d;
			d = c;
			c = LEFTROL(b, 30);
			b = a;
			a = temp;
		}

		
		for (i=16; i<20; i++)
		{
			words[i] = LEFTROL( (words[i - 3] ^ words[i - 8] ^ words[i - 14] ^ words[i - 16]), 1);
			f = (b & c) | ((~b) & d);
			temp = LEFTROL(a, 5);
			temp += f + e + k  + words[i];
		    	e = d;
			d = c;
			c = LEFTROL(b, 30);
			b = a;
			a = temp;
		}

		for (i=20; i<40; i++)
		{
  		  words[i] = LEFTROL( (words[i - 3] ^ words[i - 8] ^ words[i - 14] ^ words[i - 16]), 1);
			f=b ^ c ^ d;
			k= 0x6ED9EBA1;
			temp = LEFTROL(a, 5);
			temp += f + e + k + words[i];
			e = d;
			d = c;
			c = LEFTROL(b, 30);
			b = a;
			a = temp; 
		}

		for (i=40; i<60; i++)
		{
		    words[i] = LEFTROL( (words[i - 3] ^ words[i - 8] ^ words[i - 14] ^ words[i - 16]), 1);
			f = (b & c) | (b & d) | (c & d);
			k = 0x8F1BBCDC;
			temp = LEFTROL(a, 5);
			temp += f + e + k+ words[i];
			e = d;
			d = c;
			c = LEFTROL(b, 30);
			b = a;
			a = temp;
		}

		for (i=60; i<80; i++)
		{
		    words[i] = LEFTROL( (words[i - 3] ^ words[i - 8] ^ words[i - 14] ^ words[i - 16]), 1);
			f = b ^ c ^ d;
			k = 0xCA62C1D6;
			temp = LEFTROL(a, 5);
			temp += f + e + k + words[i];
			e = d;
			d = c;
			c = LEFTROL(b, 30);
			b = a;
			a = temp;
		}

		H0 += a;
		H1 += b;
		H2 += c;
		H3 += d;
		H4 += e;
	}

		a = H0;
		b = H1;
		c = H2;
		d = H3;
		e = H4;

	words[0] = FROM_BIG_ENDIAN(128);
	f = (b & c) | ((~b) & d);
	k = 0x5A827999;
	temp = LEFTROL(a, 5);
	temp += f + e + k + words[0];
	e = d;
	d = c;
	c = LEFTROL(b, 30);
	b = a;
	a = temp;

	for (i=1; i<15; i++)
	{   
		words[i] = 0;
		f = (b & c) | ((~b) & d);
		temp = LEFTROL(a, 5);
		temp += f + e + k + words[i];
		e = d;
		d = c;
		c = LEFTROL(b, 30);
		b = a;
		a = temp;
	}	
	

	words[15] =  dataLen*8; 
	f = (b & c) | ((~b) & d);
	temp = LEFTROL(a, 5);
	temp += f + e + k + words[15];
	e = d;
	d = c;
	c = LEFTROL(b, 30);
	b = a;
	a = temp;
	
		for (i=16; i<20; i++)
		{
			words[i] = LEFTROL( (words[i - 3] ^ words[i - 8] ^ words[i - 14] ^ words[i - 16]), 1);
			f = (b & c) | ((~b) & d);
			temp = LEFTROL(a, 5);
			temp += f + e + k + words[i];
		    	e = d;
			d = c;
			c = LEFTROL(b, 30);
			b = a;
			a = temp;
		}

		for (i=20; i<40; i++)
		{
		    words[i] = LEFTROL( (words[i - 3] ^ words[i - 8] ^ words[i - 14] ^ words[i - 16]), 1);
			f=b ^ c ^ d;
			k = 0x6ED9EBA1;
			temp = LEFTROL(a, 5);
			temp += f + e + k + words[i];
			e = d;
			d = c;
			c = LEFTROL(b, 30);
			b = a;
			a = temp; 
		}

		for (i=40; i<60; i++)
		{
			words[i] = LEFTROL( (words[i - 3] ^ words[i - 8] ^ words[i - 14] ^ words[i - 16]), 1);
			f = (b & c) | (b & d) | (c & d);
			k = 0x8F1BBCDC;
			temp = LEFTROL(a, 5);
			temp += f + e + k + words[i];
			e = d;
			d = c;
			c = LEFTROL(b, 30);
			b = a;
			a = temp;
		}

		for (i=60; i<80; i++)
		{
			words[i] = LEFTROL( (words[i - 3] ^ words[i - 8] ^ words[i - 14] ^ words[i - 16]), 1);
			f = b ^ c ^ d;
			k = 0xCA62C1D6;
			temp = LEFTROL(a, 5);
			temp += f + e + k + words[i];
			e = d;
			d = c;
			c = LEFTROL(b, 30);
			b = a;
			a = temp;
		}

		H0 += a;
		H1 += b;
		H2 += c;
		H3 += d;
		H4 += e;

		 
		int ct=0;
	md[ct++] =FROM_BIG_ENDIAN( H0);
	md[ct++] =FROM_BIG_ENDIAN( H1);
	md[ct++] =FROM_BIG_ENDIAN( H2);
	md[ct++] =FROM_BIG_ENDIAN( H3);
	md[ct++] =FROM_BIG_ENDIAN( H4);
 
}

__global__ void sha1_kernel(unsigned int *offset, unsigned char *input, unsigned char *output, unsigned int num) 
{
	int index=blockIdx.x*blockDim.x+threadIdx.x;
	if(index<num)
	{
		GPU_sha1_kernel(input+offset[index],offset[index+1]-offset[index],(unsigned int*)(output+index*FINGERPRINT_LEN));
	}
}

//the length of the data, the number of the blocks
void GPU_sha1_init(unsigned int len,unsigned int num)
{
	cudaSetDevice(0);
	cudaMalloc((void**)&gpu_input_data_s, len*sizeof(unsigned char));
	cudaMalloc((void**)&gpu_output_data_s, num*FINGERPRINT_LEN);
	cudaMalloc((void**)&gpu_offset, (num+1)*sizeof(unsigned int));
}

void GPU_sha1_destroy(void)
{
	cudaFree(gpu_input_data_s);
	cudaFree(gpu_output_data_s);
	cudaFree(gpu_offset);
}

// the max length of the block is no more than 8192(8K)
void GPU_sha1(unsigned char *input,unsigned char *output,unsigned int *offset,unsigned int num,unsigned int len)
{
	GPU_sha1_init(len,num);
	cudaMemcpy(gpu_input_data_s,input,len*sizeof(unsigned char),cudaMemcpyHostToDevice);
	cudaMemcpy(gpu_offset,offset,(num+1)*sizeof(unsigned int),cudaMemcpyHostToDevice);

	unsigned int threadNum=32;
	unsigned int blockNum=(unsigned int)(num+threadNum-1)/threadNum;
	dim3 grid(blockNum,1,1);
	dim3 threads(threadNum,1,1);
	sha1_kernel<<<grid,threads>>>(gpu_offset,gpu_input_data_s,gpu_output_data_s,num);
	cudaThreadSynchronize();

	cudaMemcpy(output,gpu_output_data_s,num*FINGERPRINT_LEN,cudaMemcpyDeviceToHost);
	GPU_sha1_destroy();
//	cudaDeviceSynchronize();
}



/*
void create_file(unsigned int len,unsigned int &num,unsigned int *chunk_offset,unsigned char *str)
{
	srand(time(NULL));	
	for(int i=0;i<len;i++)
		str[i]='a'+rand()%26;
	int next_off,offset=0;
	num=0;	
	while(offset<len)
	{
		chunk_offset[num++]=offset;
		next_off=rand()%8192;
		offset+=next_off;
	}
	chunk_offset[num]=len;
}

int main(int argc,char** argv)
{
	unsigned int len,num;
	unsigned char *input_str,*output;
	unsigned int *chunk_offset=new unsigned int[10000];
	int ch=getopt(argc,argv,"cr:");
	if(ch=='c')
	{
		printf("input len:\n");
		scanf("%d",&len);
		input_str=new unsigned char[len];
		create_file(len,num,chunk_offset,input_str);
		printf("total chunks: %d\n",num);
		output=new unsigned char[(num+1)*FINGERPRINT_LEN];
		for(int i=0;i<num;i++)
		{
			printf("%d  ",chunk_offset[i]);
			if((i+1)%10==0) printf("\n");
		}
		printf("\n");
	}else if(ch=='r')
	{
		//printf("%s\n",optarg);
		FILE *in=fopen(optarg,"r");
		fscanf(in,"%d",&len);
		printf("%d\n",len);
		input_str=new unsigned char[len];
		char tmp=fgetc(in);
		for(int i=0;i<len;i++)
		{
			fscanf(in,"%c",&input_str[i]);
			//printf(":%c",input_str[i]);
		}
		printf("%s\n",input_str);
		fscanf(in,"%d",&num);
		printf("%d\n",num);
		output=new unsigned char[(num+1)*FINGERPRINT_LEN];
		for(int i=0;i<num;i++)
		{
			fscanf(in,"%d",&chunk_offset[i]);
		}
		chunk_offset[num]=len;
		for(int i=0;i<num;i++)
		{
			printf("%d ",chunk_offset[i]);
			if((i+1)%10==0) printf("\n");
		}
		printf("\n");
	}
	//FILE *result=fopen("result.txt","w");
	GPU_sha1_init(len,num);
	GPU_sha1(input_str,output,chunk_offset,num,len);
	unsigned int *sha1_int=(unsigned int*)output;
	int j=0;
	for(int i=0;i<=num;i++)
	{
		printf("%d: ",i);
		for(int k=0;k<5;k++)
			printf("%08x ",sha1_int[j++]);
		printf("\n");
	}
	free(input_str);
	free(output);
	free(chunk_offset);
	GPU_sha1_destroy();
	return 0;
}
*/
