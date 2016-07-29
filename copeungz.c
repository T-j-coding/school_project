#include"ungzip.h"
#include<pthread.h>
#include "acsm.h"
uch* result_temp;
uch* inbuf;
ush d_buf[LIT_BUFSIZE];
uch l_buf[DIST_BUFSIZE];
unsigned int result_temp_len=0;
unsigned int last_lit=0;
unsigned int last_dist=0;
unsigned int last_decode;

//ush dd_buf[LIT_BUFSIZE];
//uch ll_buf[DIST_BUFSIZE];
//int lit =0;
//int dist=0;

char ofname[MAX_PATH_LEN];
long time_stamp;
int part_nb;
long ifile_size;
long bytes_in;
long bytes_out;
int errno;
unsigned int insize;
unsigned int inptr;
long header_bytes;
unsigned int cf_outptr;
unsigned int hufts;
int lbits=9;
int dbits=6;
uch window[2L*WSIZE];
int fragnum;
int flag;
//struct data_head* Head_buf;
ulg bb = 0;
unsigned int bk = 0;
unsigned int wp;
static int num=1;
int Empty=0;
/* ========================================================================
 *  * Table of CRC-32's of all single-byte values (made by makecrc.c)
 *   */
ulg crc_32_tab[] = {
  0x00000000L, 0x77073096L, 0xee0e612cL, 0x990951baL, 0x076dc419L,
  0x706af48fL, 0xe963a535L, 0x9e6495a3L, 0x0edb8832L, 0x79dcb8a4L,
  0xe0d5e91eL, 0x97d2d988L, 0x09b64c2bL, 0x7eb17cbdL, 0xe7b82d07L,
  0x90bf1d91L, 0x1db71064L, 0x6ab020f2L, 0xf3b97148L, 0x84be41deL,
  0x1adad47dL, 0x6ddde4ebL, 0xf4d4b551L, 0x83d385c7L, 0x136c9856L,
  0x646ba8c0L, 0xfd62f97aL, 0x8a65c9ecL, 0x14015c4fL, 0x63066cd9L,
  0xfa0f3d63L, 0x8d080df5L, 0x3b6e20c8L, 0x4c69105eL, 0xd56041e4L,
  0xa2677172L, 0x3c03e4d1L, 0x4b04d447L, 0xd20d85fdL, 0xa50ab56bL,
  0x35b5a8faL, 0x42b2986cL, 0xdbbbc9d6L, 0xacbcf940L, 0x32d86ce3L,
  0x45df5c75L, 0xdcd60dcfL, 0xabd13d59L, 0x26d930acL, 0x51de003aL,
  0xc8d75180L, 0xbfd06116L, 0x21b4f4b5L, 0x56b3c423L, 0xcfba9599L,
  0xb8bda50fL, 0x2802b89eL, 0x5f058808L, 0xc60cd9b2L, 0xb10be924L,
  0x2f6f7c87L, 0x58684c11L, 0xc1611dabL, 0xb6662d3dL, 0x76dc4190L,
  0x01db7106L, 0x98d220bcL, 0xefd5102aL, 0x71b18589L, 0x06b6b51fL,
  0x9fbfe4a5L, 0xe8b8d433L, 0x7807c9a2L, 0x0f00f934L, 0x9609a88eL,
  0xe10e9818L, 0x7f6a0dbbL, 0x086d3d2dL, 0x91646c97L, 0xe6635c01L,
  0x6b6b51f4L, 0x1c6c6162L, 0x856530d8L, 0xf262004eL, 0x6c0695edL,
  0x1b01a57bL, 0x8208f4c1L, 0xf50fc457L, 0x65b0d9c6L, 0x12b7e950L,
  0x8bbeb8eaL, 0xfcb9887cL, 0x62dd1ddfL, 0x15da2d49L, 0x8cd37cf3L,
  0xfbd44c65L, 0x4db26158L, 0x3ab551ceL, 0xa3bc0074L, 0xd4bb30e2L,
  0x4adfa541L, 0x3dd895d7L, 0xa4d1c46dL, 0xd3d6f4fbL, 0x4369e96aL,
  0x346ed9fcL, 0xad678846L, 0xda60b8d0L, 0x44042d73L, 0x33031de5L,
  0xaa0a4c5fL, 0xdd0d7cc9L, 0x5005713cL, 0x270241aaL, 0xbe0b1010L,
  0xc90c2086L, 0x5768b525L, 0x206f85b3L, 0xb966d409L, 0xce61e49fL,
  0x5edef90eL, 0x29d9c998L, 0xb0d09822L, 0xc7d7a8b4L, 0x59b33d17L,
  0x2eb40d81L, 0xb7bd5c3bL, 0xc0ba6cadL, 0xedb88320L, 0x9abfb3b6L,
  0x03b6e20cL, 0x74b1d29aL, 0xead54739L, 0x9dd277afL, 0x04db2615L,
  0x73dc1683L, 0xe3630b12L, 0x94643b84L, 0x0d6d6a3eL, 0x7a6a5aa8L,
  0xe40ecf0bL, 0x9309ff9dL, 0x0a00ae27L, 0x7d079eb1L, 0xf00f9344L,
  0x8708a3d2L, 0x1e01f268L, 0x6906c2feL, 0xf762575dL, 0x806567cbL,
 0x196c3671L, 0x6e6b06e7L, 0xfed41b76L, 0x89d32be0L, 0x10da7a5aL,
  0x67dd4accL, 0xf9b9df6fL, 0x8ebeeff9L, 0x17b7be43L, 0x60b08ed5L,
  0xd6d6a3e8L, 0xa1d1937eL, 0x38d8c2c4L, 0x4fdff252L, 0xd1bb67f1L,
 0xa6bc5767L, 0x3fb506ddL, 0x48b2364bL, 0xd80d2bdaL, 0xaf0a1b4cL,
  0x36034af6L, 0x41047a60L, 0xdf60efc3L, 0xa867df55L, 0x316e8eefL,
  0x4669be79L, 0xcb61b38cL, 0xbc66831aL, 0x256fd2a0L, 0x5268e236L,
  0xcc0c7795L, 0xbb0b4703L, 0x220216b9L, 0x5505262fL, 0xc5ba3bbeL,
  0xb2bd0b28L, 0x2bb45a92L, 0x5cb36a04L, 0xc2d7ffa7L, 0xb5d0cf31L,
  0x2cd99e8bL, 0x5bdeae1dL, 0x9b64c2b0L, 0xec63f226L, 0x756aa39cL,
  0x026d930aL, 0x9c0906a9L, 0xeb0e363fL, 0x72076785L, 0x05005713L,
  0x95bf4a82L, 0xe2b87a14L, 0x7bb12baeL, 0x0cb61b38L, 0x92d28e9bL,
  0xe5d5be0dL, 0x7cdcefb7L, 0x0bdbdf21L, 0x86d3d2d4L, 0xf1d4e242L,
  0x68ddb3f8L, 0x1fda836eL, 0x81be16cdL, 0xf6b9265bL, 0x6fb077e1L,
  0x18b74777L, 0x88085ae6L, 0xff0f6a70L, 0x66063bcaL, 0x11010b5cL,
  0x8f659effL, 0xf862ae69L, 0x616bffd3L, 0x166ccf45L, 0xa00ae278L,
  0xd70dd2eeL, 0x4e048354L, 0x3903b3c2L, 0xa7672661L, 0xd06016f7L,
  0x4969474dL, 0x3e6e77dbL, 0xaed16a4aL, 0xd9d65adcL, 0x40df0b66L,
  0x37d83bf0L, 0xa9bcae53L, 0xdebb9ec5L, 0x47b2cf7fL, 0x30b5ffe9L,
  0xbdbdf21cL, 0xcabac28aL, 0x53b39330L, 0x24b4a3a6L, 0xbad03605L,
  0xcdd70693L, 0x54de5729L, 0x23d967bfL, 0xb3667a2eL, 0xc4614ab8L,
  0x5d681b02L, 0x2a6f2b94L, 0xb40bbe37L, 0xc30c8ea1L, 0x5a05df1bL,
  0x2d02ef8dL
};
ush mask_bits[] = {
    0x0000,
    0x0001, 0x0003, 0x0007, 0x000f, 0x001f, 0x003f, 0x007f, 0x00ff,
    0x01ff, 0x03ff, 0x07ff, 0x0fff, 0x1fff, 0x3fff, 0x7fff, 0xffff
};


ulg updcrc(uch *s,unsigned n)
//    uch *s;                 /* pointer to bytes to pump through */
 //   unsigned n;             /* number of bytes in s[] */
{
    register ulg c;         /* temporary variable */

    static ulg crc = (ulg)0xffffffffL; /* shift register contents */

    if (s == NULL) {
	c = 0xffffffffL;
    } else {
	c = crc;
        if (n) do {
            c = crc_32_tab[((int)c ^ (*s++)) & 0xff] ^ (c >> 8);
        } while (--n);
    }
    crc = c;
    return c ^ 0xffffffffL;       /* (instead of ~c for 64-bit machines) */
}

static void decodelz77()
{
    register unsigned e;  /* table entry flag/number of extra bits */
  unsigned n, d;        /* length and index for copy */
  unsigned w;           /* current window position */
  w=wp;
      //lz77
      unsigned int i;
      for(i=last_decode;i<last_dist;i++)
      {
          if(d_buf[i]==0)
          {
              slide[w++] = l_buf[i];
              if (w == WSIZE)
              {
                    wp=w;
                    if (wp != 0)
                    updcrc(window, wp);
                    //printf("%s\n",window+cf_outptr);//flush_output(w);
                    result_temp_len=wp-cf_outptr;
                    result_temp=(uch*)malloc(result_temp_len);
                    memset(result_temp,0,result_temp_len);

                    memcpy(result_temp,window+cf_outptr,result_temp_len);

                    w = 0;
                    cf_outptr=0;
              }
          }
          else
          {
              d=w-(unsigned int)d_buf[i];
              n=(unsigned int)l_buf[i]+3;
              /* do the copy */
      do {
        n -= (e = (e = WSIZE - ((d &= WSIZE-1) > w ? d : w)) > n ? n : e);
#if !defined(NOMEMCPY) && !defined(DEBUG)
        if (w - d >= e)         /* (this test assumes unsigned comparison) */
        {
          memcpy(slide + w, slide + d, e);
          w += e;
          d += e;
        }
        else                      /* do it slow to avoid memcpy() overlap */
#endif /* !NOMEMCPY */
          do {
            slide[w++] = slide[d++];
	    //Tracevv((stderr, "%c", slide[w-1]));
          } while (--e);
        if (w == WSIZE)
        {
          wp=w;
          if(wp!=0)
          updcrc(window, wp);
          //printf("%s\n",window+cf_outptr);//flush_output(w);
          result_temp_len=wp-cf_outptr;
          result_temp=(uch*)malloc(result_temp_len);
          memset(result_temp,0,result_temp_len);

          memcpy(result_temp,window+cf_outptr,result_temp_len);
          w = 0;
          cf_outptr=0;
        }
      } while (n);



          }
      }
      wp = w;                       /* restore global window pointer */
     last_decode=last_dist;
}


/*static void flush_wd()//print window data
{
    if(last_lit==0)
    return;
  //  decodelz77();
	int i=0;
	for(i;i<50;++i){
	printf("%c",l_buf[i]);
	//printf("%d",d_buf[i]);
	}
	printf("flush_wd()\n");

}*/

static void waitbuf(struct data_head* head)//wait untill new fragments fill inbuf
{
	
	char size[32];
	int next = 0;
	char* p;
	struct data_head* Head_buf = head;
	struct data_segment* temp_current_block=NULL;
	//Head_buf->chunk_size -= insize;
	if(Head_buf->chunk_size > 0) 
		Head_buf->chunk_size -= inptr;
	if(Head_buf->chunk_size < 0){
	//	Head_buf->Head.flag = 0;
		Head_buf->chunk_size = 0;
	}
	if(Head_buf->current_block == NULL)
	{
	
		Empty = 1;	
		Head_buf->gzip_flag = 0;
		return ;	
	}	
	if(Head_buf->chunk_size !=0 || insize == (Head_buf->current_block->len - Head_buf->Head_size) || insize == (Head_buf->current_block->len - Head_buf->Head_size - 2)){
		temp_current_block = Head_buf->current_block;
		Head_buf->current_block = Head_buf->current_block->next;
		next = 1;
	}
	if(Head_buf->current_block!=NULL){
		//printf("waitbuf--------------------------\n%s\n",Head_buf->current_block->data);
		inbuf = (uch*) Head_buf->current_block->data;
		if(Head_buf->chunk_size >= Head_buf->current_block->len){
//			printf("step1\n");
			insize = Head_buf->current_block->len;
			inptr=0;
			Head_buf->Head_size = 0;
			}
		else if(Head_buf->chunk_size >0){
//			printf("step2\n");
			insize = Head_buf->chunk_size;
			inptr = 0;
			Head_buf->Head_size = 0;
			}
		else{
				if(next == 0)
				{
//					printf("step3\n");
					int i=0;
					if(Head_buf->current_block->len - insize > 4)
					{
						//tjy 20150911 15:55
						if((insize + Head_buf->Head_size + 2) > Head_buf->current_block->len)
						{
							Empty = 1;
							return ;
						}
						p = memmatch(size,Head_buf->current_block->data + insize + Head_buf->Head_size + 2,'\n',32,Head_buf);
						Head_buf->chunk_size = strtol(size,NULL,16);
					}
					else
					{
						insize = Head_buf->current_block->len - Head_buf->Head_size;
						return;
					}
					if(Head_buf->chunk_size == 0 )
					{
						p = memmatch(size,p,'\n',32,Head_buf);
					}
				}
				else
				{
					p = memmatch(size,Head_buf->current_block->data,'\n',32,Head_buf);
					Head_buf->Head_size = (int)strlen(size);	
					insize = 0;

				}
				Head_buf->chunk_size = strtol(size,NULL,16);
				Head_buf->Head_size = p - Head_buf->current_block->data;
		        if(Head_buf->chunk_size == 0 && Head_buf->Head_size == 2)
		    	{
//				printf("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
		     		p = memmatch(size,p,'\n',32,Head_buf);
					Head_buf->chunk_size = strtol(size,NULL,16);
		    	}
			
		    	Head_buf->Head_size = p - Head_buf->current_block->data;
			/////
			if(Head_buf->Head_size > Head_buf->current_block->len)
				Head_buf->Head_size = Head_buf->current_block->len;	
			////
			if(Head_buf->current_block->len == 2 && Head_buf->chunk_size == 0){
				Head_buf->Head_size = Head_buf->current_block->len;
			}
			if(Head_buf->chunk_size)
			{
				inbuf = (uch*) (Head_buf->current_block->data+ Head_buf->Head_size);//insize + (int)strlen(size);
				if(Head_buf->current_block->len - Head_buf->Head_size > Head_buf->chunk_size )
					insize = Head_buf->chunk_size;
				else
                	insize = Head_buf->current_block->len - Head_buf->Head_size;//insize - (int)strlen(size);
                inptr = 0;

			}
			else{
				//Head_buf->current_block = Head_buf->current_block->next;
				insize = 0;
//				printf("step5\n");
			}
		}
	}//if
	else
	{
		//Head_buf->current_block = Head_buf->last_down;
		Head_buf->current_block = temp_current_block;
		Empty = 1;	
	}
//	printf("step6\n");

}



static int huft_free(struct huft *t)//free huffman tree
//struct huft *t;         /* table to free */
/* Free the malloc'ed tables built by huft_build(), which makes a linked
   list of the tables it made, with the links in a dummy first entry of
   each table. */
{
  register struct huft *p, *q;


  /* Go through linked list, freeing from the malloced (t[-1]) address. */
  p = t;
  while (p != (struct huft *)NULL)
  {
    q = (--p)->v.t;
    free((char*)p);
    p = q;
  }
  return 0;
}

static void clear_bufs()//initialize pinter
{
    //outcnt = 0;
    insize = inptr = 0;
    bytes_in = bytes_out = 0L;
}

static int huft_build(unsigned *b, unsigned n, unsigned s, ush *d,ush *e,struct huft **t, int *m)//original code
//unsigned *b;            /* code lengths in bits (all assumed <= BMAX) */
//unsigned n;             /* number of codes (assumed <= N_MAX) */
//unsigned s;             /* number of simple-valued codes (0..s-1) */
//ush *d;                 /* list of base values for non-simple codes */
//ush *e;                 /* list of extra bits for non-simple codes */
//struct huft **t;        /* result: starting table */
//int *m;                 /* maximum lookup bits, returns actual */
/* Given a list of code lengths and a maximum table size, make a set of
   tables to decode that set of codes.  Return zero on success, one if
   the given code set is incomplete (the tables are still built in this
   case), two if the input is invalid (all zero length codes or an
   oversubscribed set of lengths), and three if not enough memory. */
{
  unsigned a;                   /* counter for codes of length k */
  unsigned c[BMAX+1];           /* bit length count table */
  unsigned f;                   /* i repeats in table every f entries */
  int g;                        /* maximum code length */
  int h;                        /* table level */
  register unsigned i;          /* counter, current code */
  register unsigned j;          /* counter */
  register int k;               /* number of bits in current code */
  int l;                        /* bits per table (returned in m) */
  register unsigned *p;         /* pointer into c[], b[], or v[] */
  register struct huft *q;      /* points to current table */
  struct huft r;                /* table entry for structure assignment */
  struct huft *u[BMAX];         /* table stack */
  unsigned v[N_MAX];            /* values in order of bit length */
  register int w;               /* bits before this table == (l * h) */
  unsigned x[BMAX+1];           /* bit offsets, then code stack */
  unsigned *xp;                 /* pointer into x */
  int y;                        /* number of dummy codes added */
  unsigned z;                   /* number of entries in current table */
//	printf("huf_build\n");

  /* Generate counts for each bit length */
  memzero(c, sizeof(c));
  p = b;  i = n;
  do {
    /*Tracecv(*p, (stderr, (n-i >= ' ' && n-i <= '~' ? "%c %d\n" : "0x%x %d\n"),
	    n-i, *p));*/
    c[*p]++;                    /* assume all entries <= BMAX */
    p++;                      /* Can't combine with above line (Solaris bug) */
  } while (--i);
  if (c[0] == n)                /* null input--all zero length codes */
  {
    *t = (struct huft *)NULL;
    *m = 0;
    return 0;
  }


  /* Find minimum and maximum length, bound *m by those */
  l = *m;
  for (j = 1; j <= BMAX; j++)
    if (c[j])
      break;
  k = j;                        /* minimum code length */
  if ((unsigned)l < j)
    l = j;
  for (i = BMAX; i; i--)
    if (c[i])
      break;
  g = i;                        /* maximum code length */
  if ((unsigned)l > i)
    l = i;
  *m = l;


  /* Adjust last length count to fill out codes, if needed */
  for (y = 1 << j; j < i; j++, y <<= 1)
    if ((y -= c[j]) < 0)
      return 2;                 /* bad input: more codes than bits */
  if ((y -= c[i]) < 0)
    return 2;
  c[i] += y;


  /* Generate starting offsets into the value table for each length */
  x[1] = j = 0;
  p = c + 1;  xp = x + 2;
  while (--i) {                 /* note that i == g from above */
    *xp++ = (j += *p++);
  }


  /* Make a table of values in order of bit lengths */
  p = b;  i = 0;
  do {
    if ((j = *p++) != 0)
      v[x[j]++] = i;
  } while (++i < n);


  /* Generate the Huffman codes and for each, make the table entries */
  x[0] = i = 0;                 /* first Huffman code is zero */
  p = v;                        /* grab values in bit order */
  h = -1;                       /* no tables yet--level -1 */
  w = -l;                       /* bits decoded == (l * h) */
  u[0] = (struct huft *)NULL;   /* just to keep compilers happy */
  q = (struct huft *)NULL;      /* ditto */
  z = 0;                        /* ditto */

  /* go through the bit lengths (k already is bits in shortest code) */
  for (; k <= g; k++)
  {
    a = c[k];
    while (a--)
    {
      /* here i is the Huffman code of length k bits for value *p */
      /* make tables up to required level */
      while (k > w + l)
      {
        h++;
        w += l;                 /* previous table always l bits */

        /* compute minimum size table less than or equal to l bits */
        z = (z = g - w) > (unsigned)l ? l : z;  /* upper limit on table size */
        if ((f = 1 << (j = k - w)) > a + 1)     /* try a k-w bit table */
        {                       /* too few codes for k-w bit table */
          f -= a + 1;           /* deduct codes from patterns left */
          xp = c + k;
          while (++j < z)       /* try smaller tables up to z bits */
          {
            if ((f <<= 1) <= *++xp)
              break;            /* enough codes to use up j bits */
            f -= *xp;           /* else deduct codes from patterns */
          }
        }
        z = 1 << j;             /* table entries for j-bit table */

        /* allocate and link in new table */
        if ((q = (struct huft *)malloc((z + 1)*sizeof(struct huft))) ==
            (struct huft *)NULL)
        {
          if (h)
            huft_free(u[0]);
          return 3;             /* not enough memory */
        }
        hufts += z + 1;         /* track memory usage */
        *t = q + 1;             /* link to list for huft_free() */
        *(t = &(q->v.t)) = (struct huft *)NULL;
        u[h] = ++q;             /* table starts after link */

        /* connect to last table, if there is one */
        if (h)
        {
          x[h] = i;             /* save pattern for backing up */
          r.b = (uch)l;         /* bits to dump before this table */
          r.e = (uch)(16 + j);  /* bits in this table */
          r.v.t = q;            /* pointer to this table */
          j = i >> (w - l);     /* (get around Turbo C bug) */
          u[h-1][j] = r;        /* connect to last table */
        }
      }

      /* set up table entry in r */
      r.b = (uch)(k - w);
      if (p >= v + n)
        r.e = 99;               /* out of values--invalid code */
      else if (*p < s)
      {
        r.e = (uch)(*p < 256 ? 16 : 15);    /* 256 is end-of-block code */
        r.v.n = (ush)(*p);             /* simple code is just the value */
	p++;                           /* one compiler does not like *p++ */
      }
      else
      {
	if(e==NULL || (*p -s) > 30)
	    return 2;
        r.e = (uch)e[*p - s];   /* non-simple--look up in lists */
        r.v.n = d[*p++ - s];
      }

      /* fill code-like entries with r */
      f = 1 << (k - w);
      for (j = i >> w; j < z; j += f)
        q[j] = r;

      /* backwards increment the k-bit code i */
      for (j = 1 << (k - 1); i & j; j >>= 1)
        i ^= j;
      i ^= j;

      /* backup over finished tables */
      while ((i & ((1 << w) - 1)) != x[h])
      {
        h--;                    /* don't need to update q */
        w -= l;
      }
    }
  }


  /* Return true (1) if we were given an incomplete table */
  return y != 0 && g != 1;
}


static int inflate_codes(struct data_head* head,struct huft *tl, struct huft *td, int bl, int bd)//decode huffmancode
//struct huft *tl, *td;   /* literal/length and distance decoder tables */
//int bl, bd;             /* number of bits decoded by tl[] and td[] */
{
  //printf("inflate_codes\n");
  register unsigned e;  /* table entry flag/number of extra bits */
  unsigned n, d;        /* length and index for copy */
  struct huft *t;       /* pointer to table entry */
  unsigned ml, md;      /* masks for bl and bd bits */
  register ulg b;       /* bit buffer */
  register unsigned   k;  /* number of bits in bit buffer */
  int loop_flag = 0;
  int loop = 0;
  /* make local copies of globals */
  b = bb;                       /* initialize bit buffer */
  k = bk;
 // e=0;
  struct data_head* Head_buf = head;
  struct data_head* head_temp=NULL;
  ush* temp_dist_buf=NULL;
  uch* temp_lit_buf=NULL;
  if(flag!=0)
	{
		b=Head_buf->Head.bb;
		k=Head_buf->Head.bk;
		e=Head_buf->Head.ee;
		t = Head_buf->Head.tt;
	}
  /* inflate the coded data */
  ml = mask_bits[bl];           /* precompute masks for speed */
  md = mask_bits[bd];
  for (;;)                      /* do until end of block */
  {
	
	if(flag==0||flag==9)
	{
		flag=9;
		Head_buf->Head.ee = e;
		NEEDBITS((unsigned)bl)
		flag=0;
	}
	if(flag==0||flag==10)
	{
		if (flag == 10 ||(e = (t = tl + ((unsigned)b & ml))->e) > 16)
			do {
					if (e == 99)
						return 1;

					DUMPBITS(t->b)
					if(flag==0)
					{
						e -= 16;
					}
					flag = 10;
					Head_buf->Head.ee=e;
					Head_buf->Head.tt =t;
					NEEDBITS(e)
					if(Head_buf->Head.tt ==NULL){
					    return 1;	
					}
					flag=0;
				} while ((e = (t = t->v.t + ((unsigned)b & mask_bits[e]))->e) > 16);
		
		//ush ttt = (ush)Head_buf->Head.tt->b;
		//b>>ttt;
		//printf("k==%d",k);
		//printf("tt==%d",ttt);
		//printf("\n");
		//if(k<ttt)
		  //return 1;
		//k-=ttt;		
		DUMPBITS((ush)t->b)
	}
    if (e == 16)                /* then it's a literal */
    {
		Head_buf->Head.ee = e;
		if(t == NULL){
			//printf("tt is null\n");
			return 1;
		}
		//ush temp = Head_buf->Head.tt->v.n;
		//Head_buf->litter_buf[Head_buf->lit++] = 'a';
		//Head_buf->lit--;
		Head_buf->litter_buf[Head_buf->lit++] = (uch)t->v.n;
		Head_buf->dist_buf[Head_buf->dist++] = 0;
		if(Head_buf->lit==LIT_BUFSIZE)
		{

			head_temp = Head_buf;	
			temp_lit_buf = Head_buf->litter_buf;
			temp_dist_buf = Head_buf->dist_buf;
			if(keyword_match(Head_buf,Head_buf->litter_buf,Head_buf->dist_buf,Head_buf->lit,Head_buf->length)){
				Head_buf->a_tcp->gk_state = GK_STATE_BLOCK;
				head_temp->Head.tt = NULL;
				Head_buf = head_temp;
				Head_buf->litter_buf = temp_lit_buf;
				Head_buf->dist_buf = temp_dist_buf;
				return 1;
			}
			Head_buf = head_temp;
		        Head_buf->litter_buf = temp_lit_buf;
			Head_buf->dist_buf = temp_dist_buf;	
			Head_buf->lit = 0;
			Head_buf->dist = 0;
			Head_buf->length = Head_buf->lit;
			if(loop++ >1 ){
				Head_buf->Head.tt = NULL;
				return 1;
			}
			//printf("the length is larger than 0x8000\n");
		 }

    }
    else                        /* it's an EOB or a length */
    {
      /* exit if end of block */
		if (e == 15)
		{
			if(Head_buf->chunk_size == insize)
			{
				Head_buf->chunk_size -= insize;
				inptr = insize;
				k=0;
				b = 0;
			}
			Head_buf->Head.flag = 0;
			huft_free(Head_buf->Head.tl);
			huft_free(Head_buf->Head.td);
			Head_buf->Head.tt = NULL;
			Head_buf->Head.tl = NULL;
			Head_buf->Head.td = NULL;
			if(Head_buf->chunk_size == 0 && (Head_buf->current_block->len - Head_buf->Head_size - insize)==7)
			{
				Head_buf->Http_flag = 0;
				Head_buf->text_flag = 0;
			}
			//printf("end of block\n");
			//printf("%s\n",Head_buf->current_block->data);
			break;
		}
		

      /* get length of block to copy */
		if(flag==0||flag==11)
		{
			Head_buf->Head.ee = e;
			flag=11;
			Head_buf->Head.tt =t;
			NEEDBITS(e)
			flag=0;
			if(Head_buf->Head.tt ==NULL){
			    return 1;	
			}
			ush temp = t->v.n +((unsigned)b&mask_bits[e])-3;
			Head_buf->litter_buf[Head_buf->lit++] = 0;
			Head_buf->lit--;
			Head_buf->litter_buf[Head_buf->lit++] = temp;//(uch)( t->v.n + ((unsigned)b & mask_bits[e]) - 3);
			if(Head_buf->lit == LIT_BUFSIZE)
			{
			   	 
				head_temp = Head_buf;	
				temp_lit_buf = Head_buf->litter_buf;
				temp_dist_buf = Head_buf->dist_buf;
				if(keyword_match(Head_buf,Head_buf->litter_buf,Head_buf->dist_buf,Head_buf->lit,Head_buf->length)){
					Head_buf->a_tcp->gk_state = GK_STATE_BLOCK;
					head_temp->Head.tt = NULL;
					Head_buf = head_temp;
					
					Head_buf->litter_buf = temp_lit_buf;
					Head_buf->dist_buf = temp_dist_buf;
					return 1;
			//	printf("\n\n************************\n\n");
				}
				Head_buf = head_temp;
				Head_buf->litter_buf = temp_lit_buf;
				Head_buf->dist_buf = temp_dist_buf;
				Head_buf->lit = 0;
				Head_buf->dist = 0;
				Head_buf->length = Head_buf->lit;
				if(loop++ > 1){
				    Head_buf->Head.tt = NULL;
				    return 1;
				}
				//printf("the length is larger than 0x8000\n");
			}	
			DUMPBITS(e)
		}
      /* decode distance of block to copy */
		if(flag==0||flag==12)
		{
			Head_buf->Head.ee = e;
			Head_buf->Head.tt = t;
			flag=12;
			NEEDBITS((unsigned)bd)
			flag=0;
		}
		if(flag==0||flag==13)
		{
			 if (flag == 13||(e = (t = td + ((unsigned)b & md))->e) > 16)
				do {
					 if (e == 99)
						 return 1;
					 if(flag==0)
					 {
						 DUMPBITS(t->b)
						 e -= 16;
					 }
					 Head_buf->Head.ee = e;
					 Head_buf->Head.tt = t;
					 flag=13;
					 NEEDBITS(e)
					 flag=0;
				   } while ((e = (t = t->v.t + ((unsigned)b & mask_bits[e]))->e) > 16);
			/*if(Head_buf->Head.tt==NULL){
				printf("tt is null\n");
				return 1;
			}*/
			DUMPBITS(t->b)
			Head_buf->Head.ee = e;
		 }
	   if(flag==0||flag==14)
		{
			flag=14;
			Head_buf->Head.tt = t;
			NEEDBITS(e)
			flag=0;
			/*ush tmp = Head_buf->Head.tt->v.n;
			ush test = Head_buf->Head.tt->v.n +(ush) ((unsigned)b & mask_bits[e]);*/
			if(Head_buf->Head.tt == NULL || t == NULL || Head_buf->dist_buf == NULL)
				return 1;
			Head_buf->dist_buf[Head_buf->dist++] = (ush)(t->v.n + ((unsigned)b & mask_bits[e]));	
		    if(Head_buf->dist==LIT_BUFSIZE)
			{
				head_temp = Head_buf;	
				temp_lit_buf = Head_buf->litter_buf;
				temp_dist_buf = Head_buf->dist_buf;
			        if(keyword_match(Head_buf,Head_buf->litter_buf,Head_buf->dist_buf,Head_buf->lit,Head_buf->length)){
					Head_buf->a_tcp->gk_state = GK_STATE_BLOCK;
					head_temp->Head.tt = NULL;
					Head_buf = head_temp;
					Head_buf->litter_buf = temp_lit_buf;
					Head_buf->dist_buf = temp_dist_buf;
					return 1;
				}
			        Head_buf = head_temp;
				Head_buf->litter_buf = temp_lit_buf;
				Head_buf->dist_buf = temp_dist_buf;
				Head_buf->lit = 0;
				Head_buf->length = Head_buf->lit;
				Head_buf->dist = 0;
				if(loop++ > 1){
					Head_buf->Head.tt = NULL;
					return 1;
				}	
			 }
			 DUMPBITS(e)
		}
	}
    if(e == 0)
    {
	loop_flag++;
	if(loop_flag > 50){
	   Head_buf->Head.tt = NULL;
	   return 1;
	}
    }
    else
	loop_flag = 0;
  }//for

  /* restore the globals from the locals */
  bb = b;                       /* restore global bit buffer */
  bk = k;
  Head_buf->Head.bb=b;
  Head_buf->Head.bk=k;
  return 0;
}



static int inflate_fixed(struct data_head* head)//original code
/* decompress an inflated type 1 (fixed Huffman codes) block.  We should
   either replace this with a custom decoder, or at least precompute the
   Huffman tables. */
{
  //printf("inflate_fixed\n");
  int i;                /* temporary variable */
  struct huft *tl;      /* literal/length code table */
  struct huft *td;      /* distance code table */
  int bl;               /* lookup bits for tl */
  int bd;               /* lookup bits for td */
  unsigned l[288];      /* length list for huft_build */
  ush* temp_dist_buf=NULL;
  uch* temp_lit_buf=NULL;
  struct data_head* Head_buf = head;
  struct data_head* head_temp = NULL;
  /* set up literal table */
  for (i = 0; i < 144; i++)
    l[i] = 8;
  for (; i < 256; i++)
    l[i] = 9;
  for (; i < 280; i++)
    l[i] = 7;
  for (; i < 288; i++)          /* make a complete, but wrong code set */
    l[i] = 8;
  bl = 7;
  if ((i = huft_build(l, 288, 257, cplens, cplext, &Head_buf->Head.tl, &bl)) != 0)
    return i;


  /* set up distance table */
  for (i = 0; i < 30; i++)      /* make an incomplete code set */
    l[i] = 5;
  bd = 5;
  if ((i = huft_build(l, 30, 0, cpdist, cpdext, &Head_buf->Head.td, &bd)) > 1)
  {
    huft_free(Head_buf->Head.tl);
    Head_buf->Head.tl = NULL;
    Head_buf->Head.td = NULL;
    Head_buf->Head.tt = NULL;
    return i;
  }
    temp_lit_buf=Head_buf->litter_buf;
    temp_dist_buf = Head_buf->dist_buf;
  /* decompress until an end-of-block code */
  if(Head_buf->Head.tl ==NULL || Head_buf->Head.td == NULL)
	return 1;
  if (inflate_codes(Head_buf,Head_buf->Head.tl, Head_buf->Head.td,bl,bd)){
		//printf("\n------------inflate_fixed fail\n");	
 		//huft_free(Head_buf->Head.tl);
		//huft_free(Head_buf->Head.td);
		Head_buf->Head.tl = NULL;
		Head_buf->Head.td = NULL;
		Head_buf->Head.tt = NULL;
		return 1;
	}
    head_temp = Head_buf;
    Head_buf->length = Head_buf->lit;
    //decode(Head_buf->litter_buf,Head_buf->dist_buf,Head_buf->length);
    if(keyword_match(Head_buf,Head_buf->litter_buf,Head_buf->dist_buf,Head_buf->lit,Head_buf->length)){
		 Head_buf->a_tcp->gk_state = GK_STATE_BLOCK;
		 if(Head_buf->Head.flag){
	         	huft_free(Head_buf->Head.tl);
		 	huft_free(Head_buf->Head.td);
    			Head_buf->Head.tl = NULL;
    			Head_buf->Head.td = NULL;
    			Head_buf->Head.tt = NULL;
		}
		 return 1;
	}
    Head_buf = head_temp; 
    Head_buf->litter_buf = temp_lit_buf;
    Head_buf->dist_buf  = temp_dist_buf;
    Head_buf->length = Head_buf->lit;	
//	Head_buf->buf->chunk_size -= inptr;
  /* free the decoding tables, return */
 // huft_free(Head_buf->Head.tl);
 // huft_free(Head_buf->Head.td);
  return 0;
}

static int inflate_dynamic(struct data_head* head)//original code
{
    //printf("inflate_dynamic()\n");
    int q=0;
    int i;                /* temporary variables */
    unsigned j = 0;
    unsigned l;           /* last length */
    unsigned m;           /* mask for bit lengths table */
    unsigned n;           /*number of lengths to get*/
 // struct huft *tl;      /* literal/length code table */
 // struct huft *td;      /* distance code table */
    int bl;               /* lookup bits for tl */
    int bd;               /* lookup bits for td */
    unsigned nb;          /* number of bit length codes */
    unsigned nl;          /* number of literal/length codes */
    unsigned nd;          /* number of distance codes */
    #ifdef PKZIP_BUG_WORKAROUND
	unsigned ll[288+32];  /* literal/length and distance code lengths */
    #else
	unsigned ll[286+30];  /* literal/length and distance code lengths */
    #endif
    register ulg b;       /* bit buffer */
    register unsigned k;  /* number of bits in bit buffer */
    /* make local bit buffer */
    struct data_head* Head_buf = head;
    b = bb;
    k = bk;
    struct data_head* head_temp;
    ush* temp_dist_buf=NULL;
    uch* temp_lit_buf=NULL;
    if(flag)
    {
		j=Head_buf->Head.j;
		nb=Head_buf->Head.nb;
		nl=Head_buf->Head.nl;
		nd=Head_buf->Head.nd;
		for(;q<316;q++)
			ll[q]=Head_buf->Head.ll[q]; 
 
		bl=Head_buf->Head.bl;
		bd=Head_buf->Head.bd;
     }
     /* read in table lengths */
    if(flag==0||flag==3)
    {
		flag=3;
        NEEDBITS(5)
		flag=0;
        nl = 257 + ((unsigned)b & 0x1f);      /* number of literal/length codes */
        DUMPBITS(5)
		Head_buf->Head.nl=nl;
    }
    if(flag==0||flag==4)
    {
		flag=4;
        NEEDBITS(5)
		flag=0;
        nd = 1 + ((unsigned)b & 0x1f);        /* number of distance codes */
        DUMPBITS(5)
		Head_buf->Head.nd=nd;
    }
    if(flag==0||flag==5)
	{
		flag=5;
		NEEDBITS(4)
		flag=0;
		nb = 4 + ((unsigned)b & 0xf);         /* number of bit length codes */
		DUMPBITS(4)
		Head_buf->Head.nb=nb;
	}
    #ifdef PKZIP_BUG_WORKAROUND
	 if (nl > 288 || nd > 32)
    #else
	 if (nl > 286 || nd > 30)
    #endif
		return 1;                   /* bad lengths */

  /* read in bit-length-code lengths */
    if(flag==0||flag==6)
    {
		for (; j < nb; j++)
		{
			flag=6;
			Head_buf->Head.j=j;
			NEEDBITS(3)
			flag=0;
			ll[border[j]] = (unsigned)b & 7;
			Head_buf->Head.ll[border[j]]=(unsigned)b & 7;
			DUMPBITS(3)

		}
		for (; j < 19; j++)
		{
			ll[border[j]] = 0;
			Head_buf->Head.ll[border[j]]=0;
		}
		Head_buf->Head.j=j;
    }

  /* build decoding table for trees--single level, 7 bit lookup */
    if(flag==0)
    {
		bl = 7;
		if ((i = huft_build(ll, 19, 19, NULL, NULL, &Head_buf->Head.tl, &bl)) != 0)
		{
			if (i == 1)
			huft_free(Head_buf->Head.tl);

    			Head_buf->Head.tl = NULL;
    			Head_buf->Head.td = NULL;
    			Head_buf->Head.tt = NULL;
			return i;                   /* incomplete code set */
		}
    }

    Head_buf->Head.bl=bl;
  /* read in literal and distance code lengths */
    n = nl + nd;
    m = mask_bits[bl];
    i = l = 0;
    if(flag==0||flag==7||flag==8)
    {
		while ((unsigned)i < n)
		{
			if(flag==0||flag==7)
			{
				flag=7;
				NEEDBITS((unsigned)bl)
				if(Head_buf->Head.tl == NULL)
					return 1;
				j = (Head_buf->Head.td = Head_buf->Head.tl + ((unsigned)b & m))->b;
				DUMPBITS(j)
				j = Head_buf->Head.td->v.n;
				Head_buf->Head.j=j;
			}
			flag=8;
			if (j < 16)
			{                 /* length of code in bits (0..15) */
				ll[i++] = l = j;          /* save last length in l */
				Head_buf->Head.ll[i-1]=j;
			}  
			else if (j == 16)           /* repeat last length 3 to 6 times */
			{

				NEEDBITS(2)

				j = 3 + ((unsigned)b & 3);
				DUMPBITS(2)

				if ((unsigned)i + j > n)
					return 1;
				while (j--)
				 {
					ll[i++] = l;
					Head_buf->Head.ll[i-1]=l;
				 }

			}
			else if (j == 17)           /* 3 to 10 zero length codes */
			{
				 NEEDBITS(3)
				 j = 3 + ((unsigned)b & 7);
				 DUMPBITS(3)
				 if ((unsigned)i + j > n)
				 return 1;
				 while (j--)
				{
					ll[i++] = 0;
					Head_buf->Head.ll[i-1]=0;
				}
					l = 0;
			}
			else                        /* j == 18: 11 to 138 zero length codes */
			{
				NEEDBITS(7)
				j = 11 + ((unsigned)b & 0x7f);
				DUMPBITS(7)
				if ((unsigned)i + j > n)
					return 1;
				while (j--)
				{
					ll[i++] = 0;
					Head_buf->Head.ll[i-1]=0;
				}
				l = 0;
	       }
	       flag=0;
	       Head_buf->Head.j=j;
		}
    }


  /* free decoding table for trees */
    if(flag < 9)
    {
		huft_free(Head_buf->Head.tl);

    		Head_buf->Head.tl = NULL;
    		Head_buf->Head.td = NULL;
    		Head_buf->Head.tt = NULL;
	 /* restore the global bit buffer */
		bb = b;
		bk = k;
		Head_buf->Head.bb=b;
		Head_buf->Head.bk=k;
	 /* build the decoding tables for literal/length and distance codes */
		bl = lbits;
		if ((i = huft_build(ll, nl, 257, cplens, cplext, &Head_buf->Head.tl, &bl)) != 0)
		{
			if (i == 1)
			{
				fprintf(stderr, " incomplete literal tree\n");
				huft_free(Head_buf->Head.tl);
    				Head_buf->Head.tl = NULL;
    				Head_buf->Head.td = NULL;
    				Head_buf->Head.tt = NULL;
			}	
			return i;                   /* incomplete code set */
		}
        bd = dbits;
		if ((i = huft_build(ll + nl, nd, 0, cpdist, cpdext, &Head_buf->Head.td, &bd)) != 0)
		{
			if (i == 1) 
			{
				fprintf(stderr, " incomplete distance tree\n");
			#ifdef PKZIP_BUG_WORKAROUND
				i = 0;
			}
			#else
			huft_free(Head_buf->Head.td);
    			Head_buf->Head.tl = NULL;
    			Head_buf->Head.td = NULL;
    			Head_buf->Head.tt = NULL;
		}
		huft_free(Head_buf->Head.tl);
    		Head_buf->Head.tl = NULL;
    		Head_buf->Head.td = NULL;
   		Head_buf->Head.tt = NULL;
		return i;                   /* incomplete code set */
	    #endif
		}
    }

    Head_buf->Head.bl=bl;
    Head_buf->Head.bd=bd;
    temp_lit_buf = Head_buf->litter_buf;
    temp_dist_buf = Head_buf->dist_buf;
    if(Head_buf->Head.tl ==NULL || Head_buf->Head.td == NULL)
	return 1;
  /* decompress until an end-of-block code */
    if (inflate_codes(Head_buf,Head_buf->Head.tl,Head_buf->Head.td,Head_buf->Head.bl,Head_buf->Head.bd)){
		//printf("\n\ninflate_codes failed\n\n");			
 		huft_free(Head_buf->Head.tl);
		huft_free(Head_buf->Head.td);
		Head_buf->Head.tt = NULL;
		Head_buf->Head.tl =NULL;
		Head_buf->Head.td = NULL;
		return 1;
		}
    head_temp = Head_buf;
    Head_buf->length = Head_buf->lit;
    //decode(Head_buf->litter_buf,Head_buf->dist_buf,Head_buf->length);
    if(keyword_match(Head_buf,Head_buf->litter_buf,Head_buf->dist_buf,Head_buf->lit,Head_buf->length)){
		 Head_buf->a_tcp->gk_state = GK_STATE_BLOCK;
		 if(Head_buf->Head.flag){
	         	huft_free(Head_buf->Head.tl);
		 	huft_free(Head_buf->Head.td);
    			Head_buf->Head.tl = NULL;
    			Head_buf->Head.td = NULL;
    			Head_buf->Head.tt = NULL;
		}
		 return 1;
	}
    Head_buf = head_temp; 
    Head_buf->litter_buf = temp_lit_buf;
    Head_buf->dist_buf  = temp_dist_buf;
    Head_buf->length = Head_buf->lit;	
  /* free the decoding tables, return */
 // huft_free(tl);
 // huft_free(td);
  return 0;
}

static int inflate_stored(struct data_head* head)//original code
{
     unsigned n;           /* number of bytes in block */
     unsigned w;           /* current window position */
     register ulg b;       /* bit buffer */
     register unsigned k;  /* number of bits in bit buffer */
     struct data_head* Head_buf = head;
     //printf("inflate_stored()%d\n",flag);
     /* make local copies of globals */
     b = bb;                       /* initialize bit buffer */
     k = bk;
     w = wp;                       /* initialize window position */
     if(flag == 17)
	n = Head_buf->Head.n;
  /* go to byte boundary */
     if(flag == 0){
        n = k & 7;
        DUMPBITS(n);
	}

     if(flag == 0 ||flag == 15)
     {
  /* get the length and its complement */
	flag = 15;
	NEEDBITS(16)
	flag = 0;
	n = ((unsigned)b & 0xffff);
	DUMPBITS(16)
      }
     if(flag == 0 || flag ==16){
	flag = 16;
	NEEDBITS(16)
	flag = 0;
	if (n != (unsigned)((~b) & 0xffff))
	    return 1;                   /* error in compressed data */
	 DUMPBITS(16)
      }
    if(flag == 0 || flag == 17){
	 /* read and output the compressed data */
	 while (n--)
	 {
	    flag = 17;
	    Head_buf->Head.n = n;
	    NEEDBITS(8)
	    flag = 0;
	    slide[w++] = (uch)b;
	    if (w == WSIZE)
	    {
		wp=w;
		if(wp!=0)
		updcrc(window, wp);
		//flush_wd();
		w = 0;
		cf_outptr=0;
	    }
	    DUMPBITS(8)
	  }
    }
/* restore the globals from the locals */
    wp = w;                       /* restore global window pointer */
    bb = b;                       /* restore global bit buffer */
    bk = k;
    Head_buf->Head.wp = wp;
    Head_buf->Head.bb = bb;
    Head_buf->Head.bk = bk;
    return 0;
}



static int inflate_block(struct data_head* head,int *e)//按huffman树编码方式解压块
//int *e;                 /* last block flag */
/* decompress an inflated block */
{
    unsigned t;           /* block type */
    register ulg b;       /* bit buffer */
    register unsigned k;  /* number of bits in bit buffer */
    //printf("inflate_block()\n");
    struct data_head* Head_buf = head;
    /* make local bit buffer */
    b = bb;
    k = bk;
    if(flag)
    {
		t=Head_buf->Head.t;
    }
  /* read in last block bit */
    if(flag==0||flag==1)
    {
		flag=1;
		NEEDBITS(1);
		flag=0;
		*e = (int)b & 1;
		Head_buf->Head.e=*e;
		DUMPBITS(1);
    }


  /* read in block type */
    if(flag==0||flag==2)
	{
		flag=2;
		NEEDBITS(2);
		flag=0;
		t = (unsigned)b & 3;
		Head_buf->Head.t=t;
		DUMPBITS(2)
    }


    /* restore the global bit buffer */
    bb = b;
    bk = k;

    /* inflate that block type */
    if (t == 2)
		return inflate_dynamic(Head_buf);
    if (t == 0)
		return inflate_stored(Head_buf);
    if (t == 1)
		return inflate_fixed(Head_buf);
    /* bad block type */
    return 2;
}


static int inflate(struct data_head* head)//循环解压压缩块直至压缩文件最后一块
{
    int e;                /* last block flag */
    int r;                /* result code */
    unsigned h;           /* maximum struct huft's malloc'ed */
    //lit = 0;
    //dist = 0;
    flag = 0; 
    struct data_head* Head_buf = head;
    flag=Head_buf->Head.flag;
    //printf("inflate()\n");
    struct huft* tl_temp = Head_buf->Head.tl;
    struct huft* td_temp = Head_buf->Head.td;
    struct huft* tt_temp = Head_buf->Head.tt; 
    if(flag)
    {	
		bb=Head_buf->Head.bb;
		bk=Head_buf->Head.bk;
		e=Head_buf->Head.e;
    }
    if(flag == 0)
    {
		bb = 0;
		bk = 0;
		Head_buf->lit = 0;
		Head_buf->dist = 0;
		Head_buf->length = Head_buf->lit;
		Head_buf->Head.tt = NULL;
		Head_buf->Head.tl = NULL;
		Head_buf->Head.td = NULL;
    }
    h = 0;
    do 
    {
        hufts = 0;
        if ((r = inflate_block(Head_buf,&e)) != 0)
          return r;
        if (hufts > h)
          h = hufts;
    } while (!e && Head_buf->Head.flag == 0);
	return 0;
}
static int get_method()//计算需要跳过的压缩块头部字节，并将unzip赋给函数指针work
{
    uch flags;     /* compression flags */
    char magic[2]; /* magic header */
    ulg stamp;     /* time stamp */
    int method;
    //printf("get_method()");
    magic[0] = (char)get_byte();
    magic[1] = (char)get_byte();
    method = -1;                 /* unknown yet */
    part_nb++;                   /* number of parts in gzip file */
    header_bytes = 0;
    if (memcmp(magic, GZIP_MAGIC, 2) == 0
        || memcmp(magic, OLD_GZIP_MAGIC, 2) == 0) 
    {
	method = (int)get_byte();
	if (method != DEFLATED) {
	    fprintf(stderr,
		    "unknown method %d -- get newer version of gzip\n",method);
	    return -1;
	}
	flags  = (uch)get_byte();
	if ((flags & ENCRYPTED) != 0) {
	    fprintf(stderr,"encrypted -- get newer version of gzip\n");

	}
	if ((flags & CONTINUATION) != 0) {
	    fprintf(stderr,"a multi-part gzip file -- get newer version of gzip\n");

	}
	if ((flags & RESERVED) != 0) {
	    fprintf(stderr,"has flags 0x%x -- get newer version of gzip\n", flags);
	   
	}
	stamp  = (ulg)get_byte();
	stamp |= ((ulg)get_byte()) << 8;
	stamp |= ((ulg)get_byte()) << 16;
	stamp |= ((ulg)get_byte()) << 24;
	if (stamp != 0) time_stamp = stamp;

	(void)get_byte();  /* Ignore extra flags for the moment */
	(void)get_byte();  /* Ignore OS type for the moment */

	if ((flags & CONTINUATION) != 0) 
	{
	    unsigned part = (unsigned)get_byte();
	    part |= ((unsigned)get_byte())<<8;

	}
	if ((flags & EXTRA_FIELD) != 0) 
	{
	    unsigned len = (unsigned)get_byte();
	    len |= ((unsigned)get_byte())<<8;
	    while (len--) (void)get_byte();
	}
	/* Get original file name if it was truncated */
	if ((flags & ORIG_NAME) != 0)
	{
	    if (part_nb > 1) {
		/* Discard the old name */
		char c; /* dummy used for NeXTstep 3.0 cc optimizer bug */
		do {c=get_byte();} while (c != 0);
	    } 
	    else {
		/* Copy the base name. Keep a directory prefix intact. */
                char *p=ofname;//char *p = basename(ofname);
                char *base = p;
		for (;;) {
		    *p = (char)get_char();
		    if (*p++ == '\0') break;
		    /*if (p >= ofname+sizeof(ofname)) {
			error("corrupted input -- file name too large");
		    }*/
		}

	    } /* no_name || to_stdout */
	} /* ORIG_NAME */


	if ((flags & COMMENTT) != 0) {
	    while (get_char() != 0) /* null */ ;
	}
	if (part_nb == 1) {
	    header_bytes = inptr + 2*sizeof(long); /* include crc and size */
	}

    }    if (method >= 0) return method;


}

void  process_gzip_data(struct data_head* head)
{
   	struct data_head* Head_buf = head; 
	if(NULL == Head_buf->a_tcp ||  NULL == Head_buf->first_down)
	{
	    return ;
	}
	//printf("process data\n");
	unsigned char litter_buf[LIT_BUFSIZE];
	unsigned short dist_buf[DIST_BUFSIZE];
	unsigned int length;
	int text_flag = 0;
	int gzip_flag = 0;
	int head_size = 0;
	int chunk_size;
	char* temp=NULL;
	struct data_segment* temp_current_block = NULL;
	//Head_buf = head;
	if(Head_buf->current_block == NULL)
	{
	
	    Head_buf->current_block = Head_buf->first_down;	
	}
	else
	{	//printf("step5\n");
		if(Head_buf->current_block->next != NULL)
		{
			 Head_buf->current_block = Head_buf->current_block->next;
		}
		else
		{
		    	//printf("step ----out\n");
			return ;
	        }
		
	}
	if(Head_buf->litter_buf == NULL || Head_buf->dist_buf == NULL || Head_buf->current_block == NULL ){
		return ;
	}
	temp = Head_buf->current_block->data;
	text_flag = Head_buf->text_flag;
	gzip_flag = Head_buf->gzip_flag;
	
	//printf("1----------------------\n%s\n",temp);
	if((strstr(Head_buf->current_block->data,"HTTP/1")!=NULL || strstr(Head_buf->current_block->data,"Content-Type:")!=NULL))
	    Head_buf->Http_flag = 0;	
	else
	    Head_buf->Http_flag = 1;
	for(;;)
	{		//for
		
	    if(!Head_buf->Http_flag)
	    {
			temp=DelHeader(Head_buf,Head_buf->current_block->data,&text_flag,&gzip_flag,&chunk_size,&head_size);
			//printf("%s",temp);
			Head_buf->text_flag = text_flag;
			Head_buf->gzip_flag = gzip_flag;
			Head_buf->chunk_size = chunk_size;
			Head_buf->Head_size = head_size;
			Head_buf->Head.flag = 0;
	    } //Http_flag==0
	    if(text_flag&&gzip_flag)
	    {		
			//printf("1----------------------\n%s\n",temp);
			//return;
			int loop =10;
			while(Head_buf->chunk_size == 0 && loop )
			{	
			   // printf("\n-------------------------------------------\n\n");						
			   /*	int i = 0; 
			    for( i = 0 ;i < Head_buf->current_block->len;i++)  
				printf("%c",Head_buf->current_block->data[i]);*/
			    char size[16];
			    loop--;
			    if(Head_buf->current_block->len <= head_size)
				 {	//printf("step5\n");
			
					temp_current_block = Head_buf->current_block;
					Head_buf->current_block = Head_buf->current_block->next;
					head_size = 0;
					if(Head_buf->current_block == NULL)
					{
						// Head_buf->current_block = Head_buf->last_down;
						Head_buf->current_block = temp_current_block;
						
						return ;
					}
					//printf("2--------------------------------\n%s\n",Head_buf->current_block);
				 }
				temp=memmatch(size,Head_buf->current_block->data + head_size,'\n',10,Head_buf);
				Head_buf->chunk_size = strtol(size,NULL,16);
				head_size += (int)strlen(size);
				if(head_size == 2 &&Head_buf->chunk_size == 0)
				 {	
					temp=memmatch(size,temp,'\n',10,Head_buf);
					head_size += (int)strlen(size);
				 }
				Head_buf->Head_size = temp - Head_buf->current_block->data;//head_size;
				if(Head_buf->chunk_size == 0 && (head_size == 3||head_size == 7))
				 {	
					Head_buf->Http_flag = 0;
					Head_buf->text_flag = 0;
					break;	
				 }
			    }
		  if(loop == 0){
			Head_buf->Http_flag = 0;
			
			return ;
		  }
				
		  if(Head_buf->current_block->len == head_size || (head_size + 2) == Head_buf->current_block->len)
		  {
			
				temp_current_block = Head_buf->current_block;
				Head_buf->current_block = Head_buf->current_block->next;	
				if(Head_buf->current_block == NULL)
				{
					Head_buf->current_block = temp_current_block;

					return ;
				}
				else
				{
					//printf("3-------------------------------------\n%s\n",Head_buf->current_block);
					temp = Head_buf->current_block->data;
					head_size = 0;
					Head_buf->Head_size = head_size;
				}
		  }
		 if(head_size == 0 && Head_buf->chunk_size < Head_buf->current_block->len)
		    head_size = Head_buf->current_block->len - Head_buf->chunk_size;
		 if(Head_buf->chunk_size > 0)
		    {
			/*for(int i = 0 ;i < Head_buf->current_block->len;i++)  
				printf("\n%c\n",Head_buf->current_block->data[i]);*/
			//printf("\nchunk_size =========%d\n",Head_buf->chunk_size);
			if(Head_buf->chunk_size < Head_buf->current_block->len - head_size)
				uncompress(Head_buf,temp,Head_buf->chunk_size,litter_buf,dist_buf,&length);
			else
				uncompress(Head_buf,temp,Head_buf->current_block->len-head_size,litter_buf,dist_buf,&length);
		    }
			///////////	deal with the chunk_size == a
		  if(Head_buf->chunk_size == 10 && Head_buf->Head.flag == 0)
		    {
				if(Head_buf->current_block != Head_buf->last_down)
					Head_buf->current_block = Head_buf->current_block->next;
				Head_buf->chunk_size = 0;
				printf("$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$\n");
				break;
		    }
			/////////////
		   if( Head_buf->chunk_size <= 0 && Head_buf->Head.flag == 0)
		    {
				Head_buf->chunk_size = 0;
				Head_buf->Http_flag = 0;
		    }//text_flag==1 and gzip_flag == 1
		    break;
		}
		else{
			/*if(strstr(Head_buf->current_block->data,"http://rrurl.cn") != NULL)
				printf("\n!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");*/
			if(Head_buf->current_block->next !=NULL && Head_buf->current_block!=Head_buf->last_down)
			    Head_buf->current_block = Head_buf->current_block->next; 
			Head_buf->Http_flag = 0;
			//printf("dont need decomprecess\n");	
			return ;	
		    }//bu yong jie ya
	    break;				
    }//for
    return ;
}
void uncompress(struct data_head* head,char* str,unsigned int len ,unsigned char* litter_buf,unsigned short* dist_buf,unsigned int* length)
{
	//printf("uncompress\n");
	//printf("thread_id%d\n",pthread_self());
	struct data_head* Head_buf = head;
	inbuf=(uch*)str;
	insize=len;
	inptr=0;
	part_nb=0;
	int method = 0;
	int inflate_error =  0 ;
	if(Head_buf->Http_flag == 0 || Head_buf->uchunk_flag)
	{
		method = get_method();
		Head_buf->Http_flag = 1;
		Head_buf->uchunk_flag = 0;
	}
	else
		method = DEFLATED;
	if(method==DEFLATED || Head_buf->Http_flag)
		inflate_error = inflate(Head_buf);
	if(inflate_error)
		Head_buf->gzip_flag = 0;
	//printf("\nuncompress---------------end\n");	
}
char*  DelHeader(struct data_head* head,char* file,int* text_flag,int* gzip_flag, int* chunk, int* head_size )
{
	char *p;
	int chunk_size;
	struct data_head* Head_buf=head;
	char size[16];
	char Head_data[2048];
	int chunk_flag=0;
	int content_length_flag =0;
	*text_flag=0;
	*gzip_flag=0;
	p=file;
	int head_length=0;
	p=memmatch(Head_data,p,'\n',2048,Head_buf);
	int loopcount = 100;
	//printf("Del----------------------\n%s\n",file);
	do{
	    //printf("%s",p);
	    //printf("\n\n");	
	    loopcount--;
	    p=memmatch(Head_data,p,'\n',2048,Head_buf);
	    if(strstr(Head_data,"text/html")!=NULL){
			
		*text_flag=1;
	    }
	    if(strstr(Head_data,"chunked")!=NULL){
		chunk_flag=1;
	    }
	    if(strstr(Head_data,"gzip")!=NULL){
		*gzip_flag=1;
		//*text_flag = 1;
	    }
	    if(strstr(Head_data,"Content-Length")!=NULL){
		content_length_flag = 1;
		char* num;
		num=(char*)malloc((strlen(Head_data)-16)*sizeof(char));
		memcpy(num,Head_data+16,strlen(Head_data)-16);
		chunk_size=strtol(num,NULL,10);
		free(num);
	    }
	    if(strstr(Head_data,"Content-Type")!=NULL){
		if(strstr(Head_data,"text/html") == NULL)
		{
		    *text_flag = 0;
		    *gzip_flag = 0;
		    return p;
		}
	    }
	  }while(strlen(Head_data)!=2 && loopcount );
	  head_length = p - file;
	 if( head_length == Head_buf->current_block->len )
	 {
	    Head_buf->Http_flag = 1;
	    Head_buf->uchunk_flag = 1;
	    *head_size = head_length;
	    if(chunk_flag || content_length_flag == 0)
			*chunk = 0;
	    else
			*chunk = chunk_size;
	    return p;
	 }
	if(chunk_flag)
	{	
	    p = memmatch(size,p,'\n',16,Head_buf);
	    chunk_size=strtol(size,NULL,16);
	    if(chunk_size == 10)
	    {
			p = memmatch(size,p + 10,'\n',16,Head_buf);
			chunk_size = strtol(size,NULL,16);
	    }
	}
	head_length = p-file;
	if(head_length > Head_buf->current_block->len)
		head_length = Head_buf->current_block->len;
	*head_size=head_length;
	*chunk=chunk_size;
	return p;
}
	

char* memmatch(char* dst, char* src ,  char c, unsigned int count,struct data_head* head)
{
    while ((count-1) && src != NULL) {
        *dst = *src;
        dst = dst + 1;
        if (*src == c || (src - head->current_block->data) >= head->current_block->len) {
            *dst = '\0';
            break;
        }
        src = src + 1;
        count--;
    }
	if(src != NULL)
		return src + 1;
	else
		return src;
}

void decode(unsigned char* litbuf,unsigned short *distbuf,int length)
{
	//printf("start decode\nlength ===================%d\n",length);
	int index = 0;
	char result[0x800000];
        int  i = 0;
	FILE* out;
	//out = fopen("out.txt","a+");
        //fprintf(out,"\n\n\n\n\n\n");
	for(index;index < length;index++)
	{
		if(distbuf[index] == 0)
			result[i++] = litbuf[index];
		else{
			int dist = distbuf[index];
			int len = litbuf[index] + 3;
			int j = 0 ;
			for(j;j < len; j++)
				result[i++] =  result[i - dist];
 
        		}
	}
	for(index = 0;index < i;index++)
	   printf("%c",result[index]);
	//fclose(out);
}	
