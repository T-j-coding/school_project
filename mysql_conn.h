#include <mysql.h>
#include <stdio.h>
#include <stdlib.h>

#include "trie.h"
#include "string_matcher.h"


struct unicode_to_string{
    char str[1024];
    char code[1024];
};

struct database_waterline;

struct my_connector;

struct my_connector* ini_my_connector();
int connector_to_mysql(struct my_connector*);
struct database_waterline* load_all_config();
void* load_config(void* arg);
void release_my_connector(struct my_connector* connector);
