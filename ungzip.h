#ifndef UNGZIP_H
#define UNGZIP_H
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <string.h>
#include<pthread.h>
#include "get_data.h"

//#define strchr            index
#define strrchr           rindex
#define memcpy(d, s, n)   bcopy((s), (d), (n))
#define memcmp(s1, s2, n) bcmp((s1), (s2), (n))
#define memzero(s, n)     bzero((s), (n))


#ifndef INBUFSIZ
#  ifdef SMALL_MEM
#    define INBUFSIZ  0x2000  /* input buffer size */
#  else
#    define INBUFSIZ  0x8000  /* input buffer size */
#  endif
#endif
#define INBUF_EXTRA  64     /* required by unlzw() */
#  define MAX_PATH_LEN   1024 /* max pathname length */

#define get_char() get_byte()
#define get_byte()  inbuf[inptr++]
#define try_byte()  inbuf[inptr++]
#define ALLOC(type, array, size) { \
    array = (type*)malloc((size)*sizeof(type)); \
    if (array == NULL) error("insufficient memory"); \
   }
//#define FREE(array) {if (array != NULL) free(array), array=NULL;}

/* Compression methods (see algorithm.doc) */
#define STORED      0
#define COMPRESSED  1
#define PACKED      2
#define LZHED       3
/* methods 4 to 7 reserved */
//int method;         /* compression method */

#define BMAX 16         /* maximum bit length of any code (16 for explode) */
#define N_MAX 288       /* maximum number of codes in any set */

#define GZIP_MAGIC     "\037\213" /* Magic header for gzip files, 1F 8B */
#define OLD_GZIP_MAGIC "\037\236" /* Magic header for gzip 0.5 = freeze 1.x */
#define DEFLATED    8
#define MAX_METHODS 9
/* gzip flag byte */
#define ASCII_FLAG   0x01 /* bit 0 set: file probably ascii text */
#define CONTINUATION 0x02 /* bit 1 set: continuation of multi-part gzip file */
#define EXTRA_FIELD  0x04 /* bit 2 set: extra field present */
#define ORIG_NAME    0x08 /* bit 3 set: original file name present */
#define COMMENTT      0x10 /* bit 4 set: file comment present */
#define ENCRYPTED    0x20 /* bit 5 set: file is encrypted */
#define RESERVED     0xC0 /* bit 6,7:   reserved */
#define NEXTBYTE() (uch)get_byte() 
#define NEEDBITS_RAW(n) {while(k<(n)){b|=((ulg)NEXTBYTE())<<k;k+=8;}}
#define DUMPBITS(n) {b>>=(n);k-=(n);}
#define NEEDBITS(n) {while(k<(n)){if(inptr<insize){b|=((ulg)NEXTBYTE())<<k;k+=8;}else{ Head_buf->Head.flag=flag;Head_buf->Head.bb=b;Head_buf->Head.bk=k;waitbuf(Head_buf);if(Empty){Empty = 0;return 0;}}}}//Head.bb=b;Head.bk=k;}}//elseif(没有数据块了)return 0;else{waitbuf();}}}
//else{flush_wd()}}}//;waitbuf();}}}
//#define NEEDBITS_dy(n) {while(k<(n)){b|=((ulg)NEXTBYTE())<<k;k+=8;}}
//{while(k<(n)){if(inptr<insize){b|=((ulg)NEXTBYTE())<<k;k+=8;}else{flush_wd()}}}//;waitbuf();}}}
//#define GETBYTE() {if(inptr<insize)inbuf[inptr++];}
//#define NEXTBYTE() (uch)GETBYTE()
//#define NEEDBITS(n) {while(k<(n)){b|=((ulg)NEXTBYTE())<<k;k+=8;}}
//#define DUMPBITS(n) {b>>=(n);k-=(n)}

#define WSIZE 0x80000
#define slide window

//extern int end=0;
//pthread_t tid;

typedef unsigned char uch;
typedef unsigned short ush;
typedef unsigned long ulg;

#define LIT_BUFSIZE  0x8000
#define DIST_BUFSIZE  LIT_BUFSIZE
//uch* result_temp;
//uch* inbuf;
//ush* d_buf;
//uch* l_buf;//store length-3
//unsigned int result_temp_len=0;
//unsigned int last_lit;    /* running index in l_buf */
//unsigned int last_dist;   /* running index in d_buf */
//unsigned int last_decode;

struct huft {
  uch e;                /* number of extra bits or operation */
  uch b;                /* number of bits in this code or subcode */
  union {
    ush n;              /* literal, length base, or distance base */
    struct huft *t;     /* pointer to next level of table */
  } v;
};
/*struct node{
	int block_size; //块长度
  struct node* next;//指向下一块的指针

	char buf[2048];//快数据
	
};*/
/*struct head{
ulg bb;
unsigned bk;
unsigned t;//block type
int e;//last block flag
int flag;
int j;
unsigned nb;
unsigned nl;
unsigned nd;
unsigned ll[286+30];
int    bl;
int bd;
unsigned ee;
};
struct HeadBuf{
        struct node* block_head;//指向压缩快数据指针
        struct HeadBuf* next;//指向下一个头
        int  Http_flag;//是否含有http头
        int  isNull;//是否有新数据
        int offset;//读取下一块数据的位置偏移
		int text_flag;
		int gzip_flag;
        struct head Head;//解压数据的头
		int chunk_size;
		struct node* current_block;
};

	
*/
/* Tables for deflate from PKZIP's appnote.txt. */
static unsigned border[] = {    /* Order of the bit length code lengths */
        16, 17, 18, 0, 8, 7, 9, 6, 10, 5, 11, 4, 12, 3, 13, 2, 14, 1, 15};
static ush cplens[] = {         /* Copy lengths for literal codes 257..285 */
        3, 4, 5, 6, 7, 8, 9, 10, 11, 13, 15, 17, 19, 23, 27, 31,
        35, 43, 51, 59, 67, 83, 99, 115, 131, 163, 195, 227, 258, 0, 0};
        /* note: see note #13 above about the 258 in this list. */
static ush cplext[] = {         /* Extra bits for literal codes 257..285 */
        0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 2, 2, 2, 2,
        3, 3, 3, 3, 4, 4, 4, 4, 5, 5, 5, 5, 0, 99, 99}; /* 99==invalid */
static ush cpdist[] = {         /* Copy offsets for distance codes 0..29 */
        1, 2, 3, 4, 5, 7, 9, 13, 17, 25, 33, 49, 65, 97, 129, 193,
        257, 385, 513, 769, 1025, 1537, 2049, 3073, 4097, 6145,
        8193, 12289, 16385, 24577};
static ush cpdext[] = {         /* Extra bits for distance codes */
        0, 0, 0, 0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6,
        7, 7, 8, 8, 9, 9, 10, 10, 11, 11,
        12, 12, 13, 13};


ulg updcrc(uch *s,unsigned n);
static void decodelz77();
static void flush_wd();
static  void  waitbuf(struct data_head* head);
static int huft_free(struct huft *t);

 static void clear_bufs();

static  int huft_build(unsigned *b,unsigned n,unsigned s,ush *d,ush *e,struct huft **t,int *m);
static int inflate_codes(struct data_head* head,struct huft *tl,struct huft *td,int bl,int bd);
static int inflate_fixed(struct data_head* head);
static int inflate_dynamic(struct data_head* head);
static int inflate_stored(struct data_head* head);
static int inflate_block(struct data_head* head,int *e);
 static int inflate();
void destory_sems();
//void decode(unsigned char* litbuf,unsigned short *distbuf,int length);
static int unzip();
static int get_method();
void *thrd_decode(void *arg);
//void ungz_initialize();
char* memungz(const char* buf,int length);
void ungz_initialize();
//void uncompress(const char *str,unsigned char *litter_buf,unsigned short *dist_buf,int*length);
void uncompress(struct data_head* head,char* str,unsigned int len,unsigned char* litter_buf,unsigned short* dist_buf,unsigned int* length);
char*  DelHeader(struct data_head* head,char* file ,int* text_flag,int* gzip_flag, int* chunk , int* head_size );
char* memmatch(char* dst, char* src , char c, unsigned int count,struct data_head* head);
struct data_head add_block();

void  process_gzip_data(struct data_head* head);
#endif
