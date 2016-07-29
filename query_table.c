#include "query_table.h"

struct logic_query_result* new_logic_query_result()
{
    struct logic_query_result* rtn = (struct logic_query_result*)malloc(sizeof(struct logic_query_result));
    if (rtn == NULL) {
	fprintf(stderr, "fatal error when allocating logic_query_result");
	exit(1);
    }

    memset(rtn, 0, sizeof(struct logic_query_result));
    
    return rtn;
}

void print_query_expression(struct query_expression* qe)
{
    int i;

    printf("#############################################\n");
    printf("query word count: %d\n", qe->query_word_num);
    for (i = 0; i < qe->query_word_num - 1; i++)
	printf("logic: %c\n", qe->logic_ops[i]);

    for (i = 0; i < qe->query_word_num; i++)
	printf("query: %s\n", qe->query[i]);
    printf("#############################################\n");
}


struct query_expression* new_query_expression()
{
    struct query_expression* rtn = (struct query_expression*)malloc(sizeof(struct query_expression));
    if (rtn == NULL) {
	fprintf(stderr, "exception, out of memory when allocating query expression!");
	return NULL;
    }
    
    memset(rtn, 0, sizeof(struct query_expression));

    return rtn;
}


void init_query_table()
{
    query_rule_table = (struct query_table*)malloc(sizeof(struct query_table));
    query_rule_table->query_count = 0;
}  
void init_query_table_N()
{
    query_rule_table_N = (struct query_table*)malloc(sizeof(struct query_table));
    query_rule_table_N->query_count = 0;
}  


int is_logic(char c)
{
    return c == '#' || c == '&';    
}


int still_has_logic(char* query)
{
    int i;

    for (i = 0; i < strlen(query); i++) {
	if (is_logic(query[i]))
	    return 1;
    }
    return 0;
}


/*
struct query_expression* parse(char* query)
{
    struct query_expression* qe = new_query_expression();

    int start = 0;
    int ops = 0;
    int word_num = 0;
    int pos;
    int i, j;
    int length=strlen(query);
    if(length>0&&length<1000)
    { 
	memcpy(qe->full_text, query, length);
    }

    for (i = 0; i < length; i++) {
	if (!still_has_logic(query + start)) {
	    int k = 0;
	    for (j = start; j < strlen(query); j++)
		qe->query[word_num][k++] = query[j];
	    word_num++;
	    break;
	}
	if (is_logic(query[i])) {
	    qe->logic_ops[ops++] = query[i];
	    int k = 0;
	    for (j = start; j < i - 1; j++) 
		qe->query[word_num][k++] = query[j];
	    word_num++;
	    start = i + 2;
	}
    }
    qe->query_word_num = word_num;

    for (i = 0 ; i < word_num; i++)
	addstr(qe->query[i]);

    return qe;
}
*/

// sunpy: 2015/09/23
// fix logic query expression space BUG
// trim space before and after expr
struct query_expression* parse(char* query)
{
    struct query_expression* qe = new_query_expression();

    int start = 0;
    int ops = 0;
    int word_num = 0;
    int pos;
    int i, j;
    int length=strlen(query);
    if(length>0&&length<1000)
    {
        memcpy(qe->full_text, query, length);
    }

    for (i = 0; i < length; i++) {
        if (!still_has_logic(query + start)) {
            int k = 0;
            while (query[start] == ' ') start++;
            for (j = start; j < strlen(query) && query[j] != ' '; j++)
                qe->query[word_num][k++] = query[j];
            word_num++;
            break;
        }
        if (is_logic(query[i])) {
            qe->logic_ops[ops++] = query[i];
            int k = 0;
            while (query[start] == ' ') start++;
            for (j = start; j <= i - 1 && query[j] != ' '; j++)
                qe->query[word_num][k++] = query[j];
            word_num++;
            start = i + 1;
        }
    }
    qe->query_word_num = word_num;

    for (i = 0 ; i < word_num; i++)
        addstr(qe->query[i]);

    return qe;
}


/*
struct query_expression* parse_N(char* query)
{
    struct query_expression* qe = new_query_expression();

    int start = 0;
    int ops = 0;
    int word_num = 0;
    int pos;
    int i, j;
    int length=strlen(query);
    if(length>0&&length<1000)
    { 
	memcpy(qe->full_text, query, length);
    }

    for (i = 0; i < length; i++) {
	if (!still_has_logic(query + start)) {
	    int k = 0;
	    for (j = start; j < strlen(query); j++)
		qe->query[word_num][k++] = query[j];
	    word_num++;
	    break;
	}
	if (is_logic(query[i])) {
	    qe->logic_ops[ops++] = query[i];
	    int k = 0;
	    for (j = start; j < i - 1; j++) 
		qe->query[word_num][k++] = query[j];
	    word_num++;
	    start = i + 2;
	}
    }
    qe->query_word_num = word_num;

    for (i = 0 ; i < word_num; i++)
	addstr_N(qe->query[i]);

    return qe;
}*/


struct query_expression* parse_N(char* query)
{
    struct query_expression* qe = new_query_expression();

    int start = 0;
    int ops = 0;
    int word_num = 0;
    int pos;
    int i, j;
    int length=strlen(query);
    if(length>0&&length<1000)
    {
        memcpy(qe->full_text, query, length);
    }

    for (i = 0; i < length; i++) {
        if (!still_has_logic(query + start)) {
            int k = 0;
            while (query[start] == ' ') start++;
            for (j = start; j < strlen(query) && query[j] != ' '; j++)
                qe->query[word_num][k++] = query[j];
            word_num++;
            break;
        }
        if (is_logic(query[i])) {
            qe->logic_ops[ops++] = query[i];
            int k = 0;
            while (query[start] == ' ') start++;
            for (j = start; j <= i - 1 && query[j] != ' '; j++)
                qe->query[word_num][k++] = query[j];
            word_num++;
            start = i + 1;
        }
    }
    qe->query_word_num = word_num;

    for (i = 0 ; i < word_num; i++)
        addstr_N(qe->query[i]);

    return qe;
}


void add_query_rule(char* query)
{
    struct query_expression* qe = parse(query);
    query_rule_table->table[query_rule_table->query_count++] = qe;        
}

void add_query_rule_N(char* query)
{
    struct query_expression* qe = parse_N(query);
    query_rule_table_N->table[query_rule_table_N->query_count++] = qe;        
}

void del_query_rule(char* query)
{
    int i;
    for (i = 0; i < query_rule_table->query_count; i++) {
        if (query_rule_table->table[i] == NULL)
            continue;

	struct query_expression* qe = query_rule_table->table[i];
        if (strcmp(qe->full_text, query) == 0) {
            query_rule_table->table[i] = NULL;
        }
    }    
}
void del_query_rule_N(char* query)
{
    int i;
    for (i = 0; i < query_rule_table_N->query_count; i++) {
        if (query_rule_table_N->table[i] == NULL)
            continue;

	struct query_expression* qe = query_rule_table_N->table[i];
        if (strcmp(qe->full_text, query) == 0) {
            query_rule_table_N->table[i] = NULL;
        }
    }    
}


int in_match_word_list(char* term, char** word_list, int word_list_count)
{
    int i;

    for (i = 0; i < word_list_count; i++) {
	if (!strcmp(word_list[i], term))
	    return 1;
    }
    return 0;
}


void populate_tf(struct query_expression* qe, int ra, char** kw)
{
    int i;

    for (i = 0; i < qe->query_word_num; i++) {
	if (in_match_word_list(qe->query[i], kw, ra))
	    qe->tf[i] = 1;
	else
	    qe->tf[i] = 0;
    }        
}


int match_(struct query_expression* qe, char* text)
{
    int ra = kw_match(text);
    char* kw[ra];
    int i;
    for (i = 0; i < ra; i++) {
        kw[i] = (char*)malloc(256 * sizeof(char));
        bzero(kw[i], 256 * sizeof(char));
    }
    get_keywords(text, kw);

    populate_tf(qe, ra, kw);
    
    if (qe->query_word_num == 1) {
	return  qe->tf[0];
    } else if (qe->query_word_num == 2) {
	if (qe->logic_ops[0] == '#') {
	    return (qe->tf[0] || qe->tf[1]);
	} else if (qe->logic_ops[0] == '&') {
	    return (qe->tf[0] && qe->tf[1]);
	}
    } else if (qe->query_word_num == 3) {
	if (qe->logic_ops[0] == '#' && qe->logic_ops[1] == '#') {
	    return (qe->tf[0] || qe->tf[1] || qe->tf[2]);
	} else if (qe->logic_ops[0] == '#' && qe->logic_ops[1] == '&') {
            return (qe->tf[0] || qe->tf[1] && qe->tf[2]);
        } else if (qe->logic_ops[0] == '&' && qe->logic_ops[1] == '#') {
            return (qe->tf[0] && qe->tf[1] || qe->tf[2]);
        } else if (qe->logic_ops[0] == '&' && qe->logic_ops[1] == '&') {
            return (qe->tf[0] && qe->tf[1] && qe->tf[2]);
        }  
    
}
free(kw);
    }
int match_N(struct query_expression* qe, char* text)
{
    int ra = kw_match_N(text);
    char* kw[ra];
    int i;
    for (i = 0; i < ra; i++) {
        kw[i] = (char*)malloc(256 * sizeof(char));
        bzero(kw[i], 256 * sizeof(char));
    }
    get_keywords_N(text, kw);

    populate_tf(qe, ra, kw);
    
    if (qe->query_word_num == 1) {
	return  qe->tf[0];
    } else if (qe->query_word_num == 2) {
	if (qe->logic_ops[0] == '#') {
	    return (qe->tf[0] || qe->tf[1]);
	} else if (qe->logic_ops[0] == '&') {
	    return (qe->tf[0] && qe->tf[1]);
	}
    } else if (qe->query_word_num == 3) {
	if (qe->logic_ops[0] == '#' && qe->logic_ops[1] == '#') {
	    return (qe->tf[0] || qe->tf[1] || qe->tf[2]);
	} else if (qe->logic_ops[0] == '#' && qe->logic_ops[1] == '&') {
            return (qe->tf[0] || qe->tf[1] && qe->tf[2]);
        } else if (qe->logic_ops[0] == '&' && qe->logic_ops[1] == '#') {
            return (qe->tf[0] && qe->tf[1] || qe->tf[2]);
        } else if (qe->logic_ops[0] == '&' && qe->logic_ops[1] == '&') {
            return (qe->tf[0] && qe->tf[1] && qe->tf[2]);
        }    
  }
    free(kw);
}



struct logic_query_result*  logic_query_match(char* text)
{
    int i;
    struct logic_query_result* rtn = new_logic_query_result();
         
    for (i = 0; i < query_rule_table->query_count; i++) {
	if (query_rule_table->table[i] == NULL)
	    continue;
	if (match_(query_rule_table->table[i], text) == 1) {
	    //print_query_expression(query_rule_table->table[i]);
	    rtn->hit_rule_list[rtn->hit_count++] = i;
	}
    }

    return rtn;
}
struct logic_query_result*  logic_query_match_N(char* text)
{
    int i;
    struct logic_query_result* rtn = new_logic_query_result();
         
    for (i = 0; i < query_rule_table_N->query_count; i++) {
	if (query_rule_table_N->table[i] == NULL)
	    continue;
	if (match_N(query_rule_table_N->table[i], text) == 1) {
	    rtn->hit_rule_list[rtn->hit_count++] = i;
	}
    }

    return rtn;
}




/*
int main()
{
    char* text = "Li 111111111111111, Hong 2222222222222222, Zhi 33333333333333, PUT 4444444444";
    char* text1 = "Li";
    char* text2 = "free";
    char* illegal = "Li Hong";
    char* text4 = "Li Hong free";

    char* query1 = "Lun & Fa # Li # Hong";
    char* query2 = "Lun & Li";
    char* query3 = "Li & Hong # Zhi";
    char* query4 = "free";

    init_ac();
    init_query_table();
    add_query_rule(query3);
    add_query_rule(query4);

    //printf("Li Hong: %d\n",match(illegal));    
    //printf("Li: %d\n", match(text1));
    //printf("free: %d\n", match(text2));
    
    printf("\n\ntext1:");
    print_readable_logic_query_result(match(text1));
    
    printf("\n\ntext2:");
    print_readable_logic_query_result(match(text2));

    printf("\n\ntext4:");
    print_readable_logic_query_result(match(text4));
}
*/
