#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "trie.h"
#include "MemoryManager.h"

#define CHILDNUM 12

struct node {
    short ref;
    int pattern_id;
    struct node* child[CHILDNUM];
};


static int global_pattern_id;
static int global_pattern_id_for_userid;
static int global_pattern_id_for_resourceid;
static struct node* trie_tree_root;
static MemoryManager* mm;

void trie_add_rule(char* subject, char* action, char* resource)
{
    if(NULL == trie_tree_root)
        {
                printf("Error, trie add rule fail\n");
                return;
    }
    int len = strlen(subject)+strlen(action)+strlen(resource);
    char *rule = (char*)malloc((3+len)*sizeof(char));
    bzero(rule,len+3);
    rule = strcat(rule,subject);
    rule = strcat(rule," ");
    rule = strcat(rule,action);
    rule = strcat(rule," ");
    rule = strcat(rule,resource);

    if (*subject == '*') {
	insert_for_resourceid(rule);
    } else if (*resource == '*') {
	insert_for_userid(rule);
    }
    
    free(rule);
    return;
}

void trie_del_rule(char* subject, char* action, char* resource)
{
    if(NULL == trie_tree_root)
    {
                printf("Error, trie delete rule fail\n");
                return;
    }
    int len = strlen(subject)+strlen(action)+strlen(resource);
    char *rule = (char*)malloc((3+len)*sizeof(char));
    bzero(rule,len+3);
    rule = strcat(rule,subject);
    rule = strcat(rule," ");
    rule = strcat(rule,action);
    rule = strcat(rule," ");
    rule = strcat(rule,resource);
        del(rule);
    free(rule);
    return;
}

int trie_chk_rule_(char* subject, char* action, char* resource, int* pattern_id)
{
    if(NULL == trie_tree_root)
    {
	printf("Error, trie check rule fail\n");
	return 0;
    }
    int found=0;
    int len = strlen(subject)+strlen(action)+strlen(resource);
    char *rule = (char*)malloc((3+len)*sizeof(char));
    bzero(rule,len+3);
    rule = strcat(rule,subject);
    rule = strcat(rule," ");
    rule = strcat(rule,action);
    rule = strcat(rule," ");
    rule = strcat(rule,resource);
//printf("trie checking:|%s|\n",rule);
    found = match(rule, pattern_id);
if(found>0)
printf("trie : bingo!\n");
    free(rule);
    return found;
}

int trie_chk_rule(char* u_id, char* s_id, int act, char* r_id, int* pattern_id_1, int* pattern_id_2)
{
    if(0==act)
	return 0;
    int a=0,b=0;
    char action[64];
    sprintf(action,"%d",act);

    if('\0'==*(u_id))
	memcpy(u_id,"NotFound",8);
    if('\0'==*(s_id))
	memcpy(s_id,"NotFound",8);
    if('\0'==*(r_id))
	memcpy(r_id,"NotFound",8);

    a = trie_chk_rule_(u_id, action, s_id, pattern_id_1);
    b = trie_chk_rule_(u_id, action, r_id, pattern_id_2);
    return a+b;
}

void addrulefromdb()
{
    int nrow=0,ncolumn;
    char *zErrMsg1, *zErrMsg2, * zErrMsg3;
    char **azResult1, **azResult2, **azResult3;
    sqlite3_get_table(db,"select Subject from idblacklist", &azResult1, &nrow, &ncolumn, &zErrMsg1);
    sqlite3_get_table(db,"select Action from idblacklist", &azResult2, &nrow, &ncolumn, &zErrMsg2);
    sqlite3_get_table(db,"select ContentId from idblacklist", &azResult3, &nrow, &ncolumn, &zErrMsg3);

    int i;
    for(i=1; i<=nrow; i++)
    {
        trie_add_rule(azResult1[i], azResult2[i], azResult3[i]);
    }
    return;
}

static struct node* new_node()
{
	malloc_count++;
    //struct node* node_ = (struct node*)alloc_from_mm(mm, sizeof(struct node));
	struct node* node_ = (struct node*)malloc(sizeof(struct node));
	bzero(node_, sizeof(struct node));
    if (node_ == NULL) {
	fprintf(stderr, "exception! out of memory !");
	exit(1);
    }
	
    node_->ref = 1;
    node_->pattern_id = -1;
	
    return node_;
}


static int c2i(char c)
{
    if (c >= '0' && c <= '9')
	return c - '0';
    else if (c == '*')
	return 10;
    else if (c == ' ')
	return 11;
}


static int is_null(struct node* node_)
{
    return node_ == NULL || node_->ref == 0;
}


static int has_child(struct node* tree)
{
    int i;

    for (i = 0; i < 12; ++i) {
	if (!is_null(tree->child[i]))
	    return 1;
    }

    return 0;
}


static void match_(struct node* tree, char* pattern, int offset, int* find, int* pattern_id)
{
    int i;

    if (pattern[offset] != 0) {  
	if (!has_child(tree)) {
	    *find |= 0;
	    return;
	}

        if (!is_null(tree->child[c2i(pattern[offset])]))   
            match_(tree->child[c2i(pattern[offset])], pattern, offset + 1, find, pattern_id);        

        if (!is_null(tree->child[c2i('*')])){    
            for (i = offset; pattern[i] != ' ' && i < strlen(pattern); ++i);
	    match_(tree->child[c2i('*')], pattern, i, find, pattern_id);  
	}
    } else {
	*find |= ((tree->pattern_id == -1) ? 0 : 1);
  
        // sunpy
        // for making distiction between case 2 and 3
        if (tree->pattern_id != -1)
            *pattern_id = tree->pattern_id;

	return;
    }
}


void init_trie_tree()
{
    global_pattern_id = 0;
    global_pattern_id_for_userid = 0;
    global_pattern_id_for_resourceid = 1;
    mm = new_memory_manager("trie");
    trie_tree_root = new_node();
	nodesize = sizeof(struct node);

    //add by yangbr, add rules from database
    addrulefromdb();
}


void destroy_trie_tree()
{
    trie_tree_root == NULL;
    free_mm(mm);
}


void insert_for_userid(char* pattern)
{
    struct node* tmp = trie_tree_root;
    int i, idx;
    for (i = 0; i < strlen(pattern); ++i) {
	idx = c2i(pattern[i]);
	if (tmp->child[idx] == NULL) {
	    tmp->child[idx] = new_node();
	} else { 
	    tmp->child[idx]->ref++;
	}
	tmp = tmp->child[idx];
    }

    tmp->pattern_id = global_pattern_id_for_userid;
    global_pattern_id_for_userid += 2;
}


void insert_for_resourceid(char* pattern)
{
    struct node* tmp = trie_tree_root;
    int i, idx;
    for (i = 0; i < strlen(pattern); ++i) {
        idx = c2i(pattern[i]);
        if (tmp->child[idx] == NULL) {
            tmp->child[idx] = new_node();
        } else {
            tmp->child[idx]->ref++;
        }
        tmp = tmp->child[idx];
    }

    tmp->pattern_id = global_pattern_id_for_resourceid;
    global_pattern_id_for_resourceid += 2;
}


void del(char* pattern)
{
    int i, idx;
    int len = strlen(pattern);
    struct node* tmp = trie_tree_root;

    for (i = 0; i < len; ++i) {
	idx = c2i(pattern[i]);
	if (is_null(tmp->child[idx]))
	    break;
	tmp = tmp->child[idx];
    }

    if (i == len && tmp->pattern_id != -1) {
	tmp = trie_tree_root;
	for (i = 0; i < len; i++) {
	    idx = c2i(pattern[i]);
	    if (tmp->child[idx]->ref >= 1)
		tmp->child[idx]->ref--;
	    tmp = tmp->child[idx];
	}

	tmp->pattern_id = -1;
    }
}


static char i2c(int i)
{
    if (i >= 0 && i <= 9)
	return i + '0';
    else if (i == 10)
	return '*';
    else if (i == 11)
	return ' ';
}


static void dfs_(struct node* tree, int offset, int depth)
{
    if (tree == NULL)
	return;

    if (offset != -1)
	printf("(%c, %d, %d, %d)\n", i2c(offset), tree->ref, tree->pattern_id, depth);

    int i;
    for (i = 0; i < 12; ++i) 
	dfs_(tree->child[i], i, depth + 1);
}


void dfs()
{
    dfs_(trie_tree_root, -1, 0);
}


int match(char* tupe, int* pattern_id)
{
    int find = 0;

    match_(trie_tree_root, tupe, 0, &find, pattern_id);

    return find;
}
