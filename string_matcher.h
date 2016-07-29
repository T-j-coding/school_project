#ifndef _string_matcher_h
#define _string_matcher_h

#include <stdio.h>
#include <regex.h>
#include <string.h>
#include <sqlite3.h>
#include "acsmx.h"

extern sqlite3 * db;
//sqlite3 * db;

static ACSM_STRUCT * A=NULL;
static ACSM_STRUCT * B=NULL;
static ACSM_STRUCT * R=NULL;
static ACSM_STRUCT * URL_A=NULL;
static ACSM_STRUCT * URL_B=NULL;
static ACSM_STRUCT * NA=NULL;
static ACSM_STRUCT * NB=NULL;
int init_ac();

regex_t regForHttp;

int addstr(const char * );
int addstr_N(const char * );
int addurl(const char * );

int delstr(const char * );
int delstr_N(const char * );
int delurl(const char * );

int kw_match(const char * );
int kw_match_N(const char * );
int url_match(const char * );

int get_request_offset(const char * s, int offsetlist[]);
int get_keywords(const char* s, char* keywords[]);
int get_keywords_N(const char* s, char* keywords[]);
#endif
