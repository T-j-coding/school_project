#include <stdio.h>
#include <sqlite3.h>
#include <string.h>
#define MAXL 500
#define sMAX 1000

char** query(char *sql, int* nrow, int* ncolumn, sqlite3* db)
{
    char *zErrMsg;
    char **azResult;
    sqlite3_get_table(db, sql, &azResult, nrow, ncolumn, &zErrMsg);
    //fprintf(stderr,"%s\n",zErrMsg);
    return azResult;
}

int exeNonQuery(char* sql, sqlite3* db)//return 0 if Ok
{
    char *zErrMsg;
    int r=sqlite3_exec(db, sql, 0, 0, &zErrMsg);
    return r;
}

int chk_idblacklist(char* Subject, char* Sname, char* Action, char* ControlTimeFrom, char* ControlTimeTo, char* ContentId, char* ContentType, sqlite3* db)
{
	//step1 : make query string
	//select * from idblacklist where (Subject='xxxxx' or Subject='*') and (Action='xxx' or Action='*') and (ContentId='xxxxx' or ContentId='*')

	char sql[sMAX] = "select * from idblacklist where";
	if(*Subject != '\0')
	{
		strcat(sql," (Subject='");
		strcat(sql,Subject);
		strcat(sql,"' or Subject='*')");
	}
	if(*Sname != '\0')
	{
		strcat(sql," and (Sname='");
		strcat(sql,Sname);
		strcat(sql,"' or Sname='*')");
	}
	if(*Action != '\0')
	{
		strcat(sql," and (Action='");
		strcat(sql,Action);
		strcat(sql,"' or Action='*')");
	}
	if(*ControlTimeFrom != '\0')
	{
		strcat(sql," and (ControlTimeFrom='");
		strcat(sql,ControlTimeFrom);
		strcat(sql,"' or ControlTimeFrom='*')");
	}
	if(*ControlTimeTo != '\0')
	{
		strcat(sql," and (ControlTimeTo='");
		strcat(sql,ControlTimeTo);
		strcat(sql,"' or ControlTimeTo='*')");
	}
	if(*ContentId != '\0')
	{
		strcat(sql," and (ContentId='");
		strcat(sql,ContentId);
		strcat(sql,"' or ContentId='*')");
	}
	if(*ContentType !='\0')
	{
		strcat(sql," and (ContentType='");
		strcat(sql,ContentType);
		strcat(sql,"' or ContentType='*')");
	}

	//step2 : query in database:
	int nrow=0,ncolumn;
	char *zErrMsg;
    char **azResult;
	//printf("sql:%s\n",sql);
	sqlite3_get_table(db, sql, &azResult, &nrow, &ncolumn, &zErrMsg);
	return nrow;		
}

int chk_rule(char* u_id, char* s_id, int p_type, char* r_id, sqlite3* db)
{
//	printf("Now check the rule\n");
	int a=0,b=0;
	char action[64];
	sprintf(action,"%d",p_type);

	a = chk_idblacklist(u_id,"\0",action,"\0","\0",s_id,"\0",db);
	b = chk_idblacklist(u_id,"\0",action,"\0","\0",r_id,"\0",db);
	//check in database now, 
	//use Trie tree later, chk_trituple()
	//if(a+b!=0)
	//	printf("Bingo!!!\n");
	//else
	//	printf("Miss!!!\n");
	return a+b;
}

int chk_keyword(char* word, sqlite3* db)
{
	//step1 : make query string
	char sql[sMAX] = "select * from keywordtable where Keyword='";
	strcat(sql,word);
	strcat(sql,"'");
		
	
	//step2 : query in database:
	int nrow=0,ncolumn;
	char *zErrMsg;
    char **azResult;
	//printf("sql:%s\n",sql);
	sqlite3_get_table(db, sql, &azResult, &nrow, &ncolumn, &zErrMsg);
	return nrow;		
}
/*
int main( )
{
    char* dbpath="/home/shutter/pcap/shutter.db";
    sqlite3 *db=NULL;
    char *zErrMsg = 0;
    int rc;
    rc = sqlite3_open(dbpath, &db);
    if( rc )
    {
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return 0;
    }

	//int n=find_db("barackobama", "100000179669235", 8, "",db);
	//int n = iskeyword("Obama", db);
	int n = chk_rule("*", "1466057540315879", 5, "22222", db);
	//int n = chk_keyword("ybrtest", db);
    printf("%d\n",n);
	sqlite3_close(db); //close database
    return 0;
}
*/

