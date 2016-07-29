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
int init_ac();
int addstr(const char * );
int delstr(const char * );
int kw_match(const char * );
int get_request_offset(const char * s, int offsetlist[]);
int get_keywords(const char* s, char* keywords[]);
#endif
