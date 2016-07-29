#ifndef LOOP_H
#define LOOP_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "http_parse.h"

#include "string_matcher.h"
#include "include.h"
#include "get_data.h"
#include "log-wxy.h"
#include "query_table.h"
#include "down_string_match.h"

extern int init_ac();
void init_loop();
void* down_loop(void *arg);
void* loop(void *arg);
struct decision_result{
    int url;
    int gk;
    int monitor;
    int trie;
};


struct location_tuple{
    unsigned short serverport;
    unsigned int serverip;
    unsigned short clientport;
    unsigned int clientip;
};

extern sqlite3 *db;
#define THREAD_NUM 3

#endif
