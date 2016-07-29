#include "string_matcher.h"

int addstr(const char* s)
{
	int length = strlen(s);
	char* string = (char*)malloc(length+1);
	bzero(string, length+1);
	memcpy(string, s, length);
	    acsmAddPattern(B, (unsigned char*)string, length);
	    A->acsmPatterns=B->acsmPatterns;
	    acsmCompile(B);
	    ACSM_STRUCT * C=A;
	    A=B;
	    B=C;
	    free(string);
	    return 1;
}

int addurl(const char* s)
{
	int length = strlen(s);
	char* string = (char*)malloc(length+1);
	bzero(string, length+1);
	memcpy(string, s, length);
	    acsmAddPattern(URL_B, (unsigned char*)string, length);
	    URL_A->acsmPatterns=URL_B->acsmPatterns;
	    acsmCompile(URL_B);
	    ACSM_STRUCT * C=URL_A;
	    URL_A=URL_B;
	    URL_B=C;
	    free(string);
	    return 1;
}

int delstr(const char* s)
{
	int length = strlen(s);
	char* string = (char*)malloc(length+1);
	bzero(string, length+1);
	memcpy(string, s, length);
	
	acsmDelPattern(B, string, length);
	free(string);

	A->acsmPatterns=B->acsmPatterns;
    acsmCompile(B);

    ACSM_STRUCT * C=A;
    A=B;
    B=C;

    return 1;
}

int add_all_kw_from_db()
{
	char* sql = "select keyword from keywordtable";
    int nrow=0,ncolumn;
    char *zErrMsg;
    char **azResult;
    sqlite3_get_table(db, sql, &azResult, &nrow, &ncolumn, &zErrMsg);
	if(ncolumn!=1 || nrow ==0)
	{
		return 0;
	}
	//if(ncolumn==1)
	else
	{
		int i;
		for(i=1; i<=nrow; i++)
		{
			addstr(azResult[i]);
		}
	}
	return 1;
}
int add_all_url_from_db()
{
	char* sql = "select url from urltable";
    int nrow=0,ncolumn;
    char *zErrMsg;
    char **azResult;
    sqlite3_get_table(db, sql, &azResult, &nrow, &ncolumn, &zErrMsg);
	if(ncolumn!=1 || nrow ==0)
	{
		return 0;
	}
	//if(ncolumn==1)
	else
	{
		int i;
		for(i=1; i<=nrow; i++)
		{
			addurl(azResult[i]);
		}
	}
	return 1;
}
	

int init_ac()
{
	A = acsmNew();
	B = acsmNew();
	URL_A=acsmNew();
	URL_B=acsmNew();
	add_all_kw_from_db();
	add_all_url_from_db();
	

	//init ac for request offset
	R = acsmNew();
	acsmAddPattern(R,(unsigned char*)"GET",strlen("GET"));
	acsmAddPattern(R,(unsigned char*)"POST",strlen("POST"));
	acsmAddPattern(R,(unsigned char*)"OPTIONS",strlen("OPTIONS"));
	acsmAddPattern(R,(unsigned char*)"HEAD",strlen("HEAD"));
	acsmAddPattern(R,(unsigned char*)"PUT",strlen("PUT"));
	acsmAddPattern(R,(unsigned char*)"CONNECT",strlen("CONNECT"));
	acsmAddPattern(R,(unsigned char*)"DELETE",strlen("DELETE"));
	acsmAddPattern(R,(unsigned char*)"TRACE",strlen("TRACE"));
	acsmCompile(R);
}

int kw_match(const char* s)
{
	int length = strlen(s);
	char* string = (char*)malloc(length+1);
	bzero(string, length+1);
	memcpy(string, s, length);
	int found = acsmSearch(A, (unsigned char*)string, length);
	free(string);
//printf("found:%d\n",found);
	return found;
}
int url_match(const char* s)
{
	int length = strlen(s);
	char* string = (char*)malloc(length+1);
	bzero(string, length+1);
	memcpy(string, s, length);
	int found = acsmSearch(URL_A, (unsigned char*)string, length);
	free(string);
//printf("found:%d\n",found);
	return found;
}

int get_request_number(const char* s)
{
	int length = strlen(s);
	char* string = (char*)malloc(length+1);
	bzero(string, length+1);
	memcpy(string, s, length);
	int found = acsmSearch(R, (unsigned char*)string, length);
	free(string);
	return found;
}	

int get_request_offset(const char* s, int offsetlist[])
{
	int length = strlen(s);
	char * string = (char*)malloc(length+1);
	bzero(string,length+1);
	memcpy(string,s,length);
	int found = acsmSearch_offset(R,(unsigned char*)string,length,offsetlist);
	free(string);
	return found;
}

int get_keywords(const char* s, char* keywords[])
{
	int length = strlen(s);
	char * string = (char*)malloc(length+1);
	bzero(string,length+1);
	memcpy(string,s,length);
	int found = acsmSearch_kw(A,(unsigned char*)string,length,keywords);
	free(string);
	return found;
}
/*
int init_db() //for test in main()
{
    char* dbpath="/home/shutter/looptest/shutter.db";
    char *zErrMsg = 0;
    int rc;
    //open the database file.If the file is not exist,it will create a file.
    rc = sqlite3_open(dbpath, &db);
    if( rc )
    {
        //fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return 0;
    }
    return 1;
}

int main()
{
	int found;
	unsigned char str[100] = "GET 111111111111111, POST 2222222222222222, GET 33333333333333, PUT 4444444444";
	if(!init_db())
		return 0;
	init_ac();
	found = get_request_number(str);
printf("request number : %d\n",found);
	int offset[found];
	get_request_offset(str,offset);
int test = 0;

printf("\n");
for(;test<found;test++)
{
printf("address:%d\n" , str+offset[test]);
printf("|%s|\n" , str+offset[test]);
}
	

		
	return 1;
}
*/
