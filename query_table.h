#include "string_matcher.h"
#include <string.h>

#define MAX_QUERY_WORD_NUM 4

struct query_expression {
    int query_word_num;
    char logic_ops[MAX_QUERY_WORD_NUM - 1];
    char query[MAX_QUERY_WORD_NUM][256];
    int tf[MAX_QUERY_WORD_NUM];
    char full_text[256];
};


struct query_table {
    int query_count;
    struct query_epxression* table[10000];
};

struct query_table* query_rule_table;
struct query_table* query_rule_table_N;
#define MAX_LOGIC_HIT_RESULT_LEN 32

struct logic_query_result {
    int hit_count;
    int hit_rule_list[MAX_LOGIC_HIT_RESULT_LEN];
};

void init_query_table();
void add_query_rule(char* query);
void add_query_rule_N(char* query);
struct logic_query_result*  logic_query_match(char* text);
struct logic_query_result*  logic_query_match_N(char* text);
