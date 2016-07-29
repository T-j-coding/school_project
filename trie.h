#ifndef TRIE_TRIE_H
#define TRIE_TRIE_H

#include <sqlite3.h>

extern sqlite3*db;
struct node;

void init_trie_tree();
void destroy_trie_tree();
void insert(char*);
void insert_for_userid(char*);
void insert_for_resourceid(char*);
void del(char*);
int match(char*, int*);
void dfs();
void trie_add_rule(char*,char*,char*);
void trie_del_rule(char*,char*,char*);
int trie_chk_rule(char*,char*,int,char*, int*, int*);


long malloc_count;
int nodesize;
#endif
