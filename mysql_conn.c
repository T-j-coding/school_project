#include "mysql_conn.h"
#include "down_string_match.h"


extern ACSMX_STRUCT* N;
extern ACSMX_STRUCT* M;
extern struct unicode_to_string utos[100000];
extern  int utos_length ;


#define SERVER_ADDR "10.128.130.205"
#define USER "root"
#define PASSWORD "qwert123"
#define DATABASE "rule"
#define PORT 3306


struct database_waterline {
    int content_monitor_config_waterline;
    int content_block_config_waterline;
    int resource_monitor_config_waterline;
    int resource_block_config_waterline;
    int url_monitor_config_waterline;
    int url_block_config_waterline;
    int user_block_config_waterline;
    int user_monitor_config_waterline;
};


struct my_connector {
    MYSQL* conn;
    char server[32];
    char user[32];
    char password[32];
    char database[32];
    int port;
};


static struct database_waterline* init_database_waterline()
{
    struct database_waterline* rtn = (struct database_waterline*)malloc(sizeof(struct database_waterline));
    if (rtn == NULL) {
        fprintf(stderr, "cannot allocate space for my_connector !!!\n");
        exit(1);
    }

    rtn->content_monitor_config_waterline = -1;
    rtn->content_block_config_waterline = -1;
    rtn->resource_monitor_config_waterline = -1;
    rtn->resource_block_config_waterline = -1;
    rtn->url_monitor_config_waterline = -1;
    rtn->url_block_config_waterline = -1;
    rtn->user_block_config_waterline = -1;
    rtn->user_monitor_config_waterline = -1;

    return rtn;
}


static struct my_connector* new_my_connector()
{
    struct my_connector* rtn = (struct my_connector*)malloc(sizeof(struct my_connector));
    if (rtn == NULL) {
        fprintf(stderr, "cannot allocate space for my_connector !!!\n");
        exit(1);
    }

    return rtn;
}


struct my_connector* ini_my_connector()
{
    struct my_connector* connector = new_my_connector();
    connector->conn = mysql_init(NULL);

    strcpy(connector->server, SERVER_ADDR);
    strcpy(connector->user, USER);
    strcpy(connector->password, PASSWORD);
    strcpy(connector->database, DATABASE);
    connector->port = PORT;

    // set character
    mysql_options(connector->conn, MYSQL_SET_CHARSET_NAME, "utf8");

    return connector;
}


int connector_to_mysql(struct my_connector* connector)
{
    if (connector->conn == NULL) {
        return 0;
    }

    if (!mysql_real_connect(connector->conn, connector->server, connector->user, connector->password, connector->database, connector->port, NULL, 0)) {
        fprintf(stderr, "connect error: %s\n", mysql_error(connector->conn));
        return 0;
    }

    return 1;
}


void release_my_connector(struct my_connector* connector)
{
    mysql_close(connector->conn);
    free(connector);
}


static int is_valid(char* state)
{
    return state[0] == '1';
}


static int is_up(char* direction)
{
    return direction[0] == '1';
}


static int is_renren(char* website)
{
    return website[0] == '1';
}


//=================================================
//  content_monitor_config | content_block_config
//  keyword      2
//  website      8
//  direction    9
//  state        10
//-------------------------------------------------
//  resource_block_config | resource_monitor_config
//  resourceID   2
//  socialAction 3
//  website      9
//  state        10
//-------------------------------------------------
//  url_block_config | url_monitor_config
//  url          2
//  website      8
//  state        9
//-------------------------------------------------
//  user_block_config | user_monitor_config
//  userID       2
//  socialAction 3
//  website      9
//  state        10
//=================================================
struct database_waterline* load_all_config() 
{
    struct database_waterline* waterline = init_database_waterline();

    MYSQL_RES *res;
    MYSQL_ROW row;

    struct my_connector* connector = ini_my_connector();
    if (0 == connector_to_mysql(connector))
        return NULL;

    // ======================================================================= content_monitor_config ===
    if (mysql_query(connector->conn, "SELECT * FROM content_monitor_config ORDER BY id")) {
       fprintf(stderr, "%s\n", mysql_error(connector->conn));
       return NULL;
    }

    res = mysql_use_result(connector->conn);

    while ((row = mysql_fetch_row(res)) != NULL) {
        if (atoi(row[0]) > waterline->content_monitor_config_waterline)
            waterline->content_monitor_config_waterline = atoi(row[0]);

        if (!is_valid(row[10]) || !is_renren(row[8]))
            continue;

        printf("content_monitor_config:%s\n", row[2]);

        /////////////////////////////////////
        add_query_rule_N(row[2]);
        if (!is_up(row[9])) {
            add_str(row[2]);
            char key[1024];
            if(!strtounicode(row[2], key))
                add_str(key);
        }
        /////////////////////////////////////

    }

    // ======================================================================= content_block_config ===
    if (mysql_query(connector->conn, "SELECT * FROM content_block_config ORDER BY id")) {
       fprintf(stderr, "%s\n", mysql_error(connector->conn));
       return NULL;
    }

    res = mysql_use_result(connector->conn);

    while ((row = mysql_fetch_row(res)) != NULL) {
        if (atoi(row[0]) > waterline->content_block_config_waterline)
            waterline->content_block_config_waterline = atoi(row[0]);

        if (!is_valid(row[10]) || !is_renren(row[8]))
            continue;

        printf("content_block_config:%s\n", row[2]);

        /////////////////////////////////////
        if (is_up(row[9])) {
            add_query_rule(row[2]);
        } else {
            add_str(row[2]);
            char key[1024];
            if(!strtounicode(row[2], key)){
                add_str(key);
                memcpy(utos[utos_length].str, row[2], strlen(row[2]));
                memcpy(utos[utos_length].code, key, strlen(key));
                utos_length++;
                printf("utos_length == %d\n",utos_length);
            }
        }
        /////////////////////////////////////
    }    


    // ======================================================================= user_block_config ===
    if (mysql_query(connector->conn, "SELECT * FROM user_block_config ORDER BY id")) {
       fprintf(stderr, "%s\n", mysql_error(connector->conn));
       return NULL;
    }

    res = mysql_use_result(connector->conn);

    while ((row = mysql_fetch_row(res)) != NULL) {
        if (atoi(row[0]) > waterline->user_block_config_waterline)
            waterline->user_block_config_waterline = atoi(row[0]);

        if (!is_valid(row[10]) || !is_renren(row[9]))
            continue;

        printf("user_block_config:%s\t%s\n", row[2], row[3]);
 
        /////////////////////////////////////
        char user_rule[256];
        bzero(user_rule, 256 * sizeof(char));
        memcpy(user_rule, row[2], strlen(row[2]));
        user_rule[strlen(user_rule)] = ' ';
        memcpy(user_rule + strlen(user_rule), row[3], strlen(row[3]));
        user_rule[strlen(user_rule)] = ' ';
        user_rule[strlen(user_rule)] = '*';
        insert_for_userid(user_rule);
        /////////////////////////////////////
    }


    // ======================================================================= user_monitor_config ===
    if (mysql_query(connector->conn, "SELECT * FROM user_monitor_config ORDER BY id")) {
       fprintf(stderr, "%s\n", mysql_error(connector->conn));
       return NULL;
    }

    res = mysql_use_result(connector->conn);

    while ((row = mysql_fetch_row(res)) != NULL) {
        if (atoi(row[0]) > waterline->user_monitor_config_waterline)
            waterline->user_monitor_config_waterline = atoi(row[0]);

        if (!is_valid(row[10]) || !is_renren(row[9]))
            continue;

        /////////////////////////////////////

        /////////////////////////////////////
    }


    // ======================================================================= resource_block_config ===
    if (mysql_query(connector->conn, "SELECT * FROM resource_block_config ORDER BY id")) {
       fprintf(stderr, "%s\n", mysql_error(connector->conn));
       return NULL;
    }

    res = mysql_use_result(connector->conn);

    while ((row = mysql_fetch_row(res)) != NULL) {
        if (atoi(row[0]) > waterline->resource_block_config_waterline)
            waterline->resource_block_config_waterline = atoi(row[0]);

        if (!is_valid(row[10]) || !is_renren(row[9]))
            continue;

        printf("resource_block_config:%s\t%s\n", row[2], row[3]);

        /////////////////////////////////////
        char resource_rule[256];
        bzero(resource_rule, 256 * sizeof(char));
        resource_rule[0] = '*';
        resource_rule[1] = ' ';
        memcpy(resource_rule + strlen(resource_rule), row[3], strlen(row[3]));
        resource_rule[strlen(resource_rule)] = ' ';
        memcpy(resource_rule + strlen(resource_rule), row[2], strlen(row[2]));
        insert_for_resourceid(resource_rule);
        /////////////////////////////////////
    }


    // ======================================================================= resource_monitor_config ===
    if (mysql_query(connector->conn, "SELECT * FROM resource_monitor_config ORDER BY id")) {
       fprintf(stderr, "%s\n", mysql_error(connector->conn));
       return NULL;
    }

    res = mysql_use_result(connector->conn);

    while ((row = mysql_fetch_row(res)) != NULL) {
        if (atoi(row[0]) > waterline->resource_monitor_config_waterline)
            waterline->resource_monitor_config_waterline = atoi(row[0]);

        if (!is_valid(row[10]) || !is_renren(row[9]))
            continue;

        /////////////////////////////////////

        /////////////////////////////////////
    }    


    // ======================================================================= url_monitor_config ===
    if (mysql_query(connector->conn, "SELECT * FROM url_monitor_config ORDER BY id")) {
       fprintf(stderr, "%s\n", mysql_error(connector->conn));
       return NULL;
    }

    res = mysql_use_result(connector->conn);

    while ((row = mysql_fetch_row(res)) != NULL) {
        if (atoi(row[0]) > waterline->url_monitor_config_waterline)
            waterline->url_monitor_config_waterline = atoi(row[0]);

        if (!is_valid(row[9]) || !is_renren(row[8]))
            continue;

        /////////////////////////////////////

        /////////////////////////////////////
    }


    // ======================================================================= url_block_config ===
    if (mysql_query(connector->conn, "SELECT * FROM url_block_config ORDER BY id")) {
       fprintf(stderr, "%s\n", mysql_error(connector->conn));
       return NULL;
    }

    res = mysql_use_result(connector->conn);

    while ((row = mysql_fetch_row(res)) != NULL) {
        if (atoi(row[0]) > waterline->url_block_config_waterline)
            waterline->url_block_config_waterline = atoi(row[0]);

        if (!is_valid(row[9]) || !is_renren(row[8]))
            continue;

        printf("url_block_config:%s\n", row[2]);

        /////////////////////////////////////
        addurl(row[2]);
        /////////////////////////////////////
    }


    release_my_connector(connector);
  
    printf("load all config done !!!\n");

    return waterline;
}


void* load_config(void* arg)
{
    struct database_waterline* waterline = (struct database_waterline*)arg;

    while (1) {
        sleep(5);

        char query_string[256];     

	MYSQL_RES *res;
	MYSQL_ROW row;

	struct my_connector* connector = ini_my_connector();
	if (0 == connector_to_mysql(connector))
	    return NULL;

	// ======================================================================= content_monitor_config ===
	memset(query_string, 0, 256 * sizeof(char));
	sprintf(query_string, "SELECT * FROM content_monitor_config WHERE id > %d ORDER BY id", waterline->content_monitor_config_waterline);
	if (mysql_query(connector->conn, query_string)) {
	   fprintf(stderr, "%s\n", mysql_error(connector->conn));
	   return NULL;
	}

	res = mysql_use_result(connector->conn);

	while ((row = mysql_fetch_row(res)) != NULL) {
	    if (atoi(row[0]) > waterline->content_monitor_config_waterline)
		waterline->content_monitor_config_waterline = atoi(row[0]);

	    if (!is_renren(row[8]))
		continue;

            printf("content_monitor_config: %s\n", row[2]);

	    /////////////////////////////////////
	    if (!is_valid(row[10])) {
		del_query_rule_N(row[2]);
		//del_str(row[2]);
	    } else {
		add_query_rule_N(row[2]);
		if (!is_up(row[9])) {
		    add_str(row[2]);
		    char key[1024];
		    if(!strtounicode(row[2], key))
			add_str(key);
		}
	    }
	    /////////////////////////////////////

	}

	// ======================================================================= content_block_config ===
	memset(query_string, 0, 256 * sizeof(char));
	sprintf(query_string, "SELECT * FROM content_block_config WHERE id > %d ORDER BY id", waterline->content_block_config_waterline);
	if (mysql_query(connector->conn, query_string)) {
	   fprintf(stderr, "%s\n", mysql_error(connector->conn));
	   return NULL;
	}

	res = mysql_use_result(connector->conn);

	while ((row = mysql_fetch_row(res)) != NULL) {
	    if (atoi(row[0]) > waterline->content_block_config_waterline)
		waterline->content_block_config_waterline = atoi(row[0]);

	    if (!is_renren(row[8]))
		continue;

            printf("content_block_config:%s\n", row[2]);

	    /////////////////////////////////////
	    if (!is_valid(row[10])) {
		del_query_rule(row[2]);
		del_str(row[2]);
	    } else {
		if (is_up(row[9])) {
		    add_query_rule(row[2]);
		} else {
		    add_str(row[2]);
		    char key[1024];
		    if(!strtounicode(row[2], key)){
			add_str(key);
			memcpy(utos[utos_length].str, row[2], strlen(row[2]));
			memcpy(utos[utos_length].code, key, strlen(key));
			utos_length++;
			printf("utos_length == %d\n",utos_length);
		    }
		}
	    }
	    /////////////////////////////////////
	}    


	// ======================================================================= user_block_config ===
	memset(query_string, 0, 256 * sizeof(char));
	sprintf(query_string, "SELECT * FROM user_block_config WHERE id > %d ORDER BY id", waterline->user_block_config_waterline);
	if (mysql_query(connector->conn, query_string)) {
	   fprintf(stderr, "%s\n", mysql_error(connector->conn));
	   return NULL;
	}

	res = mysql_use_result(connector->conn);

	while ((row = mysql_fetch_row(res)) != NULL) {
	    if (atoi(row[0]) > waterline->user_block_config_waterline)
		waterline->user_block_config_waterline = atoi(row[0]);

	    if (!is_renren(row[9]))
		continue;

            printf("user_block_config: %s\t%s\n", row[2], row[3]);
     
	    /////////////////////////////////////
	    char user_rule[256];
	    bzero(user_rule, 256 * sizeof(char));
	    memcpy(user_rule, row[2], strlen(row[2]));
	    user_rule[strlen(user_rule)] = ' ';
	    memcpy(user_rule + strlen(user_rule), row[3], strlen(row[3]));
	    user_rule[strlen(user_rule)] = ' ';
	    user_rule[strlen(user_rule)] = '*';

	    if (is_valid(row[10]))
		insert_for_userid(user_rule);
	    else
		del(user_rule);
	    /////////////////////////////////////
	}


	// ======================================================================= user_monitor_config ===
	memset(query_string, 0, 256 * sizeof(char));
	sprintf(query_string, "SELECT * FROM user_monitor_config WHERE id > %d ORDER BY id", waterline->user_monitor_config_waterline);
	if (mysql_query(connector->conn, query_string)) {
	   fprintf(stderr, "%s\n", mysql_error(connector->conn));
	   return NULL;
	}

	res = mysql_use_result(connector->conn);

	while ((row = mysql_fetch_row(res)) != NULL) {
	    if (atoi(row[0]) > waterline->user_monitor_config_waterline)
		waterline->user_monitor_config_waterline = atoi(row[0]);

	    if (!is_valid(row[10]) || !is_renren(row[9]))
		continue;

	    /////////////////////////////////////

	    /////////////////////////////////////
	}


	// ======================================================================= resource_block_config ===
	memset(query_string, 0, 256 * sizeof(char));
	sprintf(query_string, "SELECT * FROM resource_block_config WHERE id > %d ORDER BY id", waterline->resource_block_config_waterline);
	if (mysql_query(connector->conn, query_string)) {
	   fprintf(stderr, "%s\n", mysql_error(connector->conn));
	   return NULL;
	}

	res = mysql_use_result(connector->conn);

	while ((row = mysql_fetch_row(res)) != NULL) {
	    if (atoi(row[0]) > waterline->resource_block_config_waterline)
		waterline->resource_block_config_waterline = atoi(row[0]);

	    if (!is_renren(row[9]))
		continue;

            printf("resource_block_config:%s\n", row[3], row[2]);

	    /////////////////////////////////////
	    char resource_rule[256];
	    bzero(resource_rule, 256 * sizeof(char));
	    resource_rule[0] = '*';
	    resource_rule[1] = ' ';
	    memcpy(resource_rule + strlen(resource_rule), row[3], strlen(row[3]));
	    resource_rule[strlen(resource_rule)] = ' ';
	    memcpy(resource_rule + strlen(resource_rule), row[2], strlen(row[2]));

	    if (is_valid(row[10])) 
		insert_for_resourceid(resource_rule);
	    else
		del(resource_rule);
	    /////////////////////////////////////
	}


	// ======================================================================= resource_monitor_config ===
	memset(query_string, 0, 256 * sizeof(char));
	sprintf(query_string, "SELECT * FROM resource_monitor_config WHERE id > %d ORDER BY id", waterline->resource_monitor_config_waterline);
	if (mysql_query(connector->conn, query_string)) {
	   fprintf(stderr, "%s\n", mysql_error(connector->conn));
	   return NULL;
	}

	res = mysql_use_result(connector->conn);

	while ((row = mysql_fetch_row(res)) != NULL) {
	    if (atoi(row[0]) > waterline->resource_monitor_config_waterline)
		waterline->resource_monitor_config_waterline = atoi(row[0]);

	    if (!is_valid(row[10]) || !is_renren(row[9]))
		continue;


	    /////////////////////////////////////

	    /////////////////////////////////////
	}    


	// ======================================================================= url_monitor_config ===
	memset(query_string, 0, 256 * sizeof(char));
	sprintf(query_string, "SELECT * FROM url_monitor_config WHERE id > %d ORDER BY id", waterline->url_monitor_config_waterline);
	if (mysql_query(connector->conn, query_string)) {
	   fprintf(stderr, "%s\n", mysql_error(connector->conn));
	   return NULL;
	}

	res = mysql_use_result(connector->conn);

	while ((row = mysql_fetch_row(res)) != NULL) {
	    if (atoi(row[0]) > waterline->url_monitor_config_waterline)
		waterline->url_monitor_config_waterline = atoi(row[0]);

	    if (!is_valid(row[9]) || !is_renren(row[8]))
		continue;

	    /////////////////////////////////////

	    /////////////////////////////////////
	}


	// ======================================================================= url_block_config ===
	memset(query_string, 0, 256 * sizeof(char));
	sprintf(query_string, "SELECT * FROM url_block_config WHERE id > %d ORDER BY id", waterline->url_block_config_waterline);
	if (mysql_query(connector->conn, query_string)) {
	   fprintf(stderr, "%s\n", mysql_error(connector->conn));
	   return NULL;
	}

	res = mysql_use_result(connector->conn);

	while ((row = mysql_fetch_row(res)) != NULL) {
	    if (atoi(row[0]) > waterline->url_block_config_waterline)
		waterline->url_block_config_waterline = atoi(row[0]);

	    if (!is_renren(row[8]))
		continue;

            printf("usl_block_config:%s\n", row[2]);

	    /////////////////////////////////////
	    if (is_valid(row[9]))
		addurl(row[2]);
	    else
		delurl(row[2]);
	    /////////////////////////////////////
	}


        release_my_connector(connector);

        printf("load config once !!! \n");
    }
}
