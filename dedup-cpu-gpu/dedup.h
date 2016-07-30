typedef unsigned int sha1_type[5];  
typedef unsigned int u_int;

#define HASH_LEN 0x10010

struct input_buf{
	int buf_size;
	int fill_buf_len;
	unsigned char *buf;
	input_buf(){
		buf_size=256*1024*1024;
		buf=new unsigned char[buf_size];
		fill_buf_len=0;
	}
};

struct mate_node{
	sha1_type sha1_val;
	long long chunk_offset;
	u_int chunk_size;
	struct mate_node *next;
};

struct hash_index_type{
	u_int node_num;
	struct mate_node *next;
	hash_index_type(){
		node_num=0;
		next=NULL;
	}
};

