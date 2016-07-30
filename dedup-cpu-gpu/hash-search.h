//#include "dedup.h"

int get_sha1_hash(const sha1_type sha1){
	return sha1[0] & 0x0000FFFF;
}

bool compare_sha1(const sha1_type srcsha1,const sha1_type detsha1){
	for(int i=0;i<5;i++){
		if(srcsha1[i]!=detsha1[i]) return false;
	}
	return true;
}

void copysha1(sha1_type detsha1,const sha1_type srcsha1){
	for(int i=0;i<5;i++)
		detsha1[i]=srcsha1[i];
}

bool chunk_exist(const sha1_type sha1,struct hash_index_type *hash_ind){
	int index=get_sha1_hash(sha1);
	if(hash_ind[index].node_num==0) return false;
	struct mate_node *node;
	node=hash_ind[index].next;
	while(node!=NULL){
		if(compare_sha1(node->sha1_val,sha1)) return true;
		node=node->next;
	}
	return false;
}

void add_new_chunk(FILE *fp,const struct input_buf *buf,const unsigned int *offset,int off_ind,const sha1_type sha1,struct hash_index_type *hash_ind,long long &f_offset){
	struct mate_node *node=new struct mate_node;
	int ind=get_sha1_hash(sha1);
	copysha1(node->sha1_val,sha1);
	node->chunk_offset=f_offset;
	node->chunk_size=offset[off_ind+1]-offset[off_ind];
	node->next=hash_ind[ind].next;
	hash_ind[ind].next=node;
	hash_ind[ind].node_num++;
	fwrite(buf->buf+offset[off_ind],node->chunk_size,1,fp);
	f_offset+=node->chunk_size;
}

static struct hash_index_type hash_ind[HASH_LEN];
static long long f_offset;

void hash_search(FILE *fp,const struct input_buf *buf ,const unsigned int *offset,const sha1_type *sha1_list, unsigned int num){
//	struct hash_index_type hash_ind[HASH_LEN];
//	FILE *fp=fopen(chunk_file_path,"w");
//	long long f_offset=0;
	for(int i=0;i<num;i++){
		if(!chunk_exist(sha1_list[i],hash_ind)){
			add_new_chunk(fp,buf,offset,i,sha1_list[i],hash_ind,f_offset);
		}
	}
//	fclose(fp);
}
