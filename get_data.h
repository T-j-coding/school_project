#ifndef GET_DATA_H
#define GET_DATA_H

#include <stdint.h>
#include <pthread.h> 
#include <unistd.h>
#include <stdio.h>
#include "nids.h"

#define DATA_CACHE_TABLE_SIZE 100003

#define WE_WANT_DATA_RECEIVED_BY_A_CLIENT
#define WE_WANT_DATA_RECEIVED_BY_A_SERVER

struct data_segment
{
    uint32_t len;
    char *data;
    struct data_segment *next;
};
struct head{
        unsigned long bb;
        unsigned bk;
        unsigned t;
        int e;
        int flag;
        int j;
        unsigned nb;
        unsigned nl;
        unsigned nd;
        unsigned ll[286+30];
        unsigned bl;
        unsigned bd;
        unsigned ee;
        unsigned wp;
        unsigned n;
        struct huft *tl;
        struct huft *td;
        struct huft *tt;
};
struct data_head
{
    struct tcp_stream *a_tcp;
    struct data_segment* first_up;
    struct data_segment* last_up;
    struct data_segment* temp_up;
    int offset_up;
    struct data_segment* first_down;
    struct data_segment* last_down;
    struct data_segment* temp_down;
    int offset_down;
    pthread_mutex_t tcp_stream_lock_up;
    pthread_mutex_t tcp_stream_lock_down;
    struct data_head* next;

    int special_flag;
    struct head Head;
    struct data_segment* current_block;
    int Http_flag;
    int text_flag;
    int gzip_flag;
    int uchunk_flag;
    int chunk_size;
    int Head_size;
    int sup_flag;
    int length;
    int dist;
    int lit;
    unsigned short *dist_buf;
    unsigned char  *litter_buf;
    int pthread_flag;
};

int system_init();
void system_run();

extern struct data_head data_cache_table[DATA_CACHE_TABLE_SIZE];
extern pthread_t pid_up_stream;
int gk_method;
#endif
