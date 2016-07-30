#define DSPT 128*1024
#define CUT_PER_T 64
#define OFS_LEN 256*512
#define PRIME_BASE 13
#define SOUT_BASE 13*13*13
#define MOD_SUM 0x00001FFF
#define OFS_NULL 0x7FFFFFFF

#define FROM_BIG_ENDIAN(v)                                          \
 ((v & 0xff) << 24) | ((v & 0xff00) << 8) | ((v & 0xff0000) >> 8) |  \
		((v & 0xff000000) >> 24)                              \

#define LEFTROL( v, n)  (v << n) | (v >> (32 - n))
#define FINGERPRINT_LEN 20
#define MAX_DATA_LEN 8192

void GPU_sha1(unsigned char *input,unsigned char *output,unsigned int *offset,unsigned int num,unsigned int len);
void GPU_rabin(int min_len,int max_len,int sign,unsigned char *input,unsigned int *offset,unsigned int &num,unsigned int len);
