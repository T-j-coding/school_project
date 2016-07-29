 
#ifndef ACSMU_H
#define ACSMU_H
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "get_data.h"
//#include <vector>
/*
*   Prototypes
*/
#define ALPHABET_SIZE    256     
#define MAXLEN 256

#define ACSM_FAIL_STATE   -1     
#define MAXWORD 30

//using namespace std;
typedef struct acsm_pattern {      

	struct  acsm_pattern *next;
	unsigned char         *patrn;
	int      n;
	//int		 nmatch;

} ACSMX_PATTERN;


typedef struct  {    

	/* Next state - based on input character */
	int      NextState[ ALPHABET_SIZE ];  

	/* Failure state - used while building NFA & DFA  */
	int      FailState;   

	/* List of patterns that end here, if any */
	ACSMX_PATTERN *MatchList;   

}ACSMX_STATETABLE; 


/*
* State machine Struct
*/
typedef struct {

	int acsmMaxStates;  
	int acsmNumStates;  

	ACSMX_PATTERN    * acsmPatterns;
	ACSMX_STATETABLE * acsmStateTable;
	int*  dept; 	
//	vector<ACSM_STATETABLE> acsmStateTable;
//	vector<int> dept;

}ACSMX_STRUCT;

/*
*   Prototypes
*/
ACSMX_STRUCT * acsmnew ();

int acsmdelPattern( ACSMX_STRUCT* p, char * pat, int n);
int acsmaddPattern( ACSMX_STRUCT* p, unsigned char * pat, int n);
int acsmcompile ( ACSMX_STRUCT* acsm );
//int acsmSearch ( ACSM_STRUCT * acsm,unsigned char * T, int n, int (*Match) (ACSM_PATTERN * pattern,ACSM_PATTERN * mlist, int nline,int index));
//int acsmSearch (ACSM_STRUCT * acsm, unsigned char *Tx, int n);
void acsmfree ( ACSMX_STRUCT* acsm );
void AddPatternStates(ACSMX_STRUCT * acsm,ACSMX_PATTERN* p);
int fsm(ACSMX_STRUCT* acsm,int state,char e,int* out,char* keyword);
int keyword_match(struct data_head* head, unsigned char l[0x8000],unsigned short d[0x8000],int len,int offset);
int acch(ACSMX_STRUCT* acsm,unsigned char l[0x8000],unsigned short dbuf[0x8000],int offset ,int last_lit,char* keyword,int* text_len );
#endif
 
