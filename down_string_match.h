#ifndef DOWN_STRING_MATCH_H
#define DOWN_STRING_MATCH_H
#include <stdio.h>
#include <regex.h>
#include <sqlite3.h>
#include "acsm.h"
extern sqlite3 * db;
//static ACSMX_STRUCT* M=NULL;
//static ACSMX_STRUCT* N=NULL;
//static ACSMX_STRUCT* P=NULL;
ACSMX_STRUCT* down_ac( );
int add_str(const char* );
int del_str(const char*);

#endif


