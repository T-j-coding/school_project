#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "acsm.h"
#include <sys/time.h>
#include "ungzip.h"
#include "down_string_match.h"
#include "loop.h"
#include "rule_accepter.h"
#include "mysql_conn.h"
#define MAXWORD 30
#define CDepth 4
//#define WSIZE 0X8000
#define LIT_BUFSIZE 0x8000
extern ACSMX_STRUCT* M;
extern ACSMX_STRUCT* N;
//extern struct unicode_to_string utos;
struct unicode_to_string utos[100000]; 
int utos_length = 0;
//extern int utos_length;
char win[WSIZE];
int st[WSIZE];
/* Malloc the AC Memory
*/ 
static void *AC_MALLOC (int n) 
{
	void *p;
	p = malloc (n);

	return p;
}

/*
*Free the AC Memory
*/ 
static void AC_FREE (void *p) 
{
	if (p)
		free (p);
}


/*
*    Simple QUEUE NODE
*/ 
typedef struct _qnode
{
	int state;
	struct _qnode *next;
}QNODE;

/*
*    Simple QUEUE Structure
*/ 
typedef struct _queue
{
	QNODE * head, *tail;
	int count;
}QUEUE;

/*
*Init the Queue
*/ 
static void queue_init (QUEUE * s) 
{
	s->head = s->tail = 0;
	s->count = 0;
}


/*
*  Add Tail Item to queue
*/ 
static void queue_add (QUEUE * s, int state) 
{
	QNODE * q;
	/*Queue is empty*/
	if (!s->head)
	{
		q = s->tail = s->head = (QNODE *) AC_MALLOC (sizeof (QNODE));
		/*if malloc failed,exit the problom*/
		q->state = state;
		q->next = 0; /*Set the New Node's Next Null*/
	}
	else
	{
		q = (QNODE *) AC_MALLOC (sizeof (QNODE));
		q->state = state;
		q->next = 0;
		/*Add the new Node into the queue*/
		s->tail->next = q;
		/*set the new node is the Queue's Tail*/
		s->tail = q;
	}
	s->count++;
}


/*
*  Remove Head Item from queue
*/ 
static int queue_remove (QUEUE * s) 
{
	int state = 0;
	QNODE * q;
	/*Remove A QueueNode From the head of the Queue*/
	if (s->head)
	{
		q = s->head;
		state = q->state;
		s->head = s->head->next;
		s->count--;

		/*If Queue is Empty,After Remove A QueueNode*/
		if (!s->head)
		{
			s->tail = 0;
			s->count = 0;
		}
		/*Free the QueNode Memory*/
		AC_FREE (q);
	}
	return state;
}


/*
*Return The count of the Node in the Queue
*/ 
static int queue_count (QUEUE * s) 
{
	return s->count;
}


/*
*Free the Queue Memory
*/ 
static void queue_free (QUEUE * s) 
{
	while (queue_count (s))
	{
		queue_remove (s);
	}
}


/*
*  Add a pattern to the list of patterns terminated at this state.
*  Insert at front of list.
*/ 
static void AddMatchListEntry (ACSMX_STRUCT * acsm, int state, ACSMX_PATTERN * px) 
{
	ACSMX_PATTERN * p;
	p = (ACSMX_PATTERN *) AC_MALLOC (sizeof (ACSMX_PATTERN));
	memcpy (p, px, sizeof (ACSMX_PATTERN));

	/*Add the new pattern to the pattern  list*/
	p->next = acsm->acsmStateTable[state].MatchList;
	acsm->acsmStateTable[state].MatchList = p;
}

/* 
* Add Pattern States
*/ 
void AddPatternStates (ACSMX_STRUCT * acsm, ACSMX_PATTERN * p) 
{
	unsigned char *pattern;
	int state=0, next, n;
	n = p->n; /*The number of alpha in the pattern string*/
	pattern = p->patrn;
    
	/* 
	*  Match up pattern with existing states
	*/ 
	for (; n > 0; pattern++, n--)
	{
		next = acsm->acsmStateTable[state].NextState[*pattern];
		if (next == ACSM_FAIL_STATE)
			break;
		state = next;
	}

	/*
	*   Add new states for the rest of the pattern bytes, 1 state per byte
	*/ 
	int i;
	for (; n > 0; pattern++, n--)
	{
		acsm->acsmNumStates++;
/*  del by ybr, moidify for c
		ACSMX_STATETABLE state_p;
		state_p.MatchList=0;
        	//acsm->acsmStateTable.push_back(state_p);
        	acsm->acsmStateTable[acsm->acsmNumStates] = state_p;
	    for (i = 0; i < ALPHABET_SIZE; i++)
	    {
		    acsm->acsmStateTable[acsm->acsmNumStates].NextState[i] = ACSM_FAIL_STATE;
	    }*/
		acsm->acsmStateTable[state].NextState[*pattern] = acsm->acsmNumStates;
		//printf("dept_size=============%d",sizeof(acsm->dept));
	//	printf("acsmNumstates========%d",acsm->acsmNumStates);
	//	printf("dept__value==========%d",acsm->dept[state]);
		acsm->dept[acsm->acsmNumStates] = acsm->dept[state]+1;
		state = acsm->acsmNumStates;
	}
	/*Here,An accept state,just add into the MatchListof the state*/
	AddMatchListEntry (acsm, state, p);
}


/*
*   Build Non-Deterministic Finite Automata
*/ 
static void build_DFA (ACSMX_STRUCT * acsm) 
{
	int r, s;
	int i;
	QUEUE q, *queue = &q;
	ACSMX_PATTERN * mlist=0;
	ACSMX_PATTERN * px=0;

	/* Init a Queue */ 
	queue_init (queue);

	/* Add the state 0 transitions 1st */
	/*1st depth Node's FailState is 0, fail(x)=0 */
	for (i = 0; i < ALPHABET_SIZE; i++)
	{
		s = acsm->acsmStateTable[0].NextState[i];
		if (s)
		{
			queue_add (queue, s);
			acsm->acsmStateTable[s].FailState = 0;
		}
	}

	/* Build the fail state transitions for each valid state */ 
	while (queue_count (queue) > 0)
	{
		r = queue_remove (queue);

		/* Find Final States for any Failure */ 
		for (i = 0; i < ALPHABET_SIZE; i++)
		{
			int fs, next;
			/*** Note NextState[i] is a const variable in this block ***/
			if ((s = acsm->acsmStateTable[r].NextState[i]) != ACSM_FAIL_STATE)
			{
				queue_add (queue, s);
				fs = acsm->acsmStateTable[r].FailState;

				/* 
				*  Locate the next valid state for 'i' starting at s 
				*/ 
				/**** Note the  variable "next" ****/
				/*** Note "NextState[i]" is a const variable in this block ***/
				while ((next=acsm->acsmStateTable[fs].NextState[i]) ==
					ACSM_FAIL_STATE)
				{
					fs = acsm->acsmStateTable[fs].FailState;
				}

				/*
				*  Update 's' state failure state to point to the next valid state
				*/ 
				acsm->acsmStateTable[s].FailState = next;

                                
                                ACSMX_PATTERN* pat = acsm->acsmStateTable[next].MatchList;
                                for (; pat != NULL; pat = pat->next)
                                {
                                    AddMatchListEntry(acsm, s, pat);    
                                }
			}
			else
			{
				acsm->acsmStateTable[r].NextState[i] =
					acsm->acsmStateTable[acsm->acsmStateTable[r].FailState].NextState[i];
			}
		}
	}

	/* Clean up the queue */ 
	queue_free (queue);
}


/*
* Init the acsm DataStruct
*/ 
ACSMX_STRUCT * acsmnew () 
{
	ACSMX_STRUCT * p;
	p = (ACSMX_STRUCT *) AC_MALLOC (sizeof (ACSMX_STRUCT));
	if (p)
		memset (p, 0, sizeof (ACSMX_STRUCT));
	return p;
}


/*
*   Add a pattern to the list of patterns for this state machine
*/ 
int acsmaddPattern (ACSMX_STRUCT * p, unsigned char *pat, int n) 
{
	ACSMX_PATTERN * plist;
	plist = (ACSMX_PATTERN *) AC_MALLOC (sizeof (ACSMX_PATTERN));
	plist->patrn = (unsigned char *) AC_MALLOC (n+1);
	memset(plist->patrn+n,0,1);
	memcpy (plist->patrn, pat, n);
	plist->n = n;
	//plist->nmatch=0;
	/*Add the pattern into the pattern list*/
	plist->next = p->acsmPatterns;
	p->acsmPatterns = plist;

	return 0;
}



int acsmdelPattern (ACSMX_STRUCT * p, char *pat, int n) 
{
	ACSMX_PATTERN * plist;
	ACSMX_PATTERN * culist;
	
	//plist->nmatch=0;
	//Add the pattern into the pattern list
	plist = p->acsmPatterns;
	if(strcmp(pat,(char*)(plist->patrn))==0)
		p->acsmPatterns=plist->next;
	else if(plist){
		while(plist->next)
		{
			culist=plist->next;
			if(!strcmp(pat,(char*)(culist->patrn)))
			{
				plist->next=culist->next;
				free(culist);
				break;
			}
			plist=culist;
		}
	}

	return 0;
}

/*
*   Compile State Machine
*/ 
int acsmcompile (ACSMX_STRUCT * acsm) 
{
	int i, k;
	//acsm->dept[0]=0;
	ACSMX_PATTERN * plist;
	//printf("compile acsm======\n");
	/* Count number of states */ 
	acsm->acsmMaxStates = 1; /*State 0*/
	for (plist = acsm->acsmPatterns; plist != NULL; plist = plist->next)
	{
		acsm->acsmMaxStates += plist->n;
	}

	acsm->acsmStateTable = (ACSMX_STATETABLE *) AC_MALLOC (sizeof (ACSMX_STATETABLE) * acsm->acsmMaxStates);
	acsm->dept = (int *) malloc(sizeof(int)*acsm->acsmMaxStates);
//	MEMASSERT (acsm->acsmStateTable, "acsmCompile");
	memset (acsm->acsmStateTable, 0,sizeof (ACSMX_STATETABLE) * acsm->acsmMaxStates);
	memset(acsm->dept,0,sizeof(int)*acsm->acsmMaxStates);
	
	/* Initialize state zero as a branch */ 
	acsm->acsmNumStates = 0;
/*	
//del by ybr, modify to c*/
/*	ACSMX_STATETABLE state_p;
	state_p.MatchList=0;
    	//acsm->acsmStateTable.push_back(state_p);
    	acsm->acsmStateTable[0] = state_p;
	acsm->dept[0] = 0;
	for (i = 0; i < ALPHABET_SIZE; i++)
	{
		acsm->acsmStateTable[0].NextState[i] = ACSM_FAIL_STATE;
	}
*/
	
	/* Initialize all States NextStates to FAILED */ 
	for (k = 0; k < acsm->acsmMaxStates; k++)
	{
		for (i = 0; i < ALPHABET_SIZE; i++)
		{
			acsm->acsmStateTable[k].NextState[i] = ACSM_FAIL_STATE;
		}
	}
	/* This is very import */
	/* Add each Pattern to the State Table */ 
	for (plist = acsm->acsmPatterns; plist != NULL; plist = plist->next)
	{
		//printf("add pattern states");
		AddPatternStates (acsm, plist);
	}

	/* Set all failed state transitions which from state 0 to return to the 0'th state */ 
	for (i = 0; i < ALPHABET_SIZE; i++)
	{
		if (acsm->acsmStateTable[0].NextState[i] == ACSM_FAIL_STATE)
		{
			acsm->acsmStateTable[0].NextState[i] = 0;
		}
	}

	/* Build the NFA  */ 
	build_DFA (acsm);
	{
   //     std::vector<ACSM_STATETABLE>(acsm->acsmStateTable).swap(acsm->acsmStateTable);
	}
	return 0;
}

/*
void writeLog_kw(std::string word)
{
	LogTable lt;
	lt.Keyword=word;
	char time_str[30];
	time_t now_time;
    now_time = time(NULL);
	struct tm *p;
	p=localtime(&now_time);
	sprintf(time_str,"%d.%d.%d,%d:%d:%d", (1900+p->tm_year),(1+p->tm_mon),p->tm_mday, p->tm_hour, p->tm_min, p->tm_sec);
	lt.LogTime=time_str;
	Pz_kwTable kwtable;
	Pz_kw myPz_kw;
	kwtable=myPz_kw.getPz_kwByKw(word);
	lt.PZConfigId=kwtable.PZConfigId;
	lt.PZOperator=kwtable.PZOperator;
	lt.PZOperateTime=kwtable.PZOperateTime;
	lt.PZControlTimeFrom=kwtable.PZControlTimeFrom;
	lt.PZControlTimeTo=kwtable.PZControlTimeTo;
	lt.PZControlType=kwtable.PZControlType;
	
	Log mylog;
	mylog.addLog(lt);
}*/

/*
*   Search Text or Binary Data for Pattern matches
*/ 
/*int acsmSearch (ACSM_STRUCT * acsm, unsigned char *Tx, int n) 
{
	
	
	int state;
	ACSM_PATTERN * mlist;
	unsigned char *Tend;
	
    ACSM_STATETABLE * StateTable = acsm->acsmStateTable;
	

	int nfound = 0; //Number of the found(matched) patten string
	unsigned char *T;
	int index;
    
	
	T = Tx;
	Tend = T + n;

	for (state = 0; T < Tend; T++)
	{
		state = StateTable[state].NextState[*T];

		// State is a accept state? 
		if( StateTable[state].MatchList != NULL )
		{
			for( mlist=StateTable[state].MatchList; mlist!=NULL;mlist=mlist->next )
			{
				//Get the index  of the Match Pattern String in  the Text
				index = T - mlist->n + 1 - Tx;

				//mlist->nmatch++;
				nfound++;
				
				//when match the keyword,write the log
				
				//writeLog_kw((char*)mlist->patrn);
				
				//printf("Match KeyWord %s at %d char\n",mlist->patrn,index);
				
				
			}
		}
	}

    
	return nfound;
}

*/
/*
*   improvment of acsmSearch()
*   get all the keyword offsets into an array (modified by yangbr)
*/

/*int acsmSearch_offset (ACSM_STRUCT * acsm, unsigned char *Tx, int n, int offsetlist[] )
{
	int state;
	ACSM_PATTERN * mlist;
	unsigned char *Tend;

    ACSM_STATETABLE * StateTable = acsm->acsmStateTable;


	int nfound = 0; //Number of the found(matched) patten string
	unsigned char *T;
	int index = 0;


	T = Tx;
	Tend = T + n;

	for (state = 0; T < Tend; T++)
	{
		state = StateTable[state].NextState[*T];

		// State is a accept state?
		if( StateTable[state].MatchList != NULL )
		{

			for( mlist=StateTable[state].MatchList; mlist!=NULL;mlist=mlist->next )
			{
				//Get the index  of the Match Pattern String in  the Text
				index = T - mlist->n + 1 - Tx;

				//mlist->nmatch++;
				offsetlist[nfound] = index;
				
				nfound++;
				
				//printf("Match KeyWord %s at %d char\n",mlist->patrn,index);
				//Debugger()<<"Match KeyWord "<< mlist->patrn <<" at " << index << " char\n";
                //vptr.push_back((char*)(index + Tx));
			}
		}
	}
	return nfound;
}*/
int fsm(ACSMX_STRUCT * acsm,int state,char e,int* out,char* keyword)//return next state
{
    ACSMX_STATETABLE* StateTable = (acsm->acsmStateTable);
    ACSMX_PATTERN * mlist;
        state = StateTable[state].NextState[(unsigned char)e];
    if( StateTable[state].MatchList != NULL )
        {
            *out=1;
            for( mlist=StateTable[state].MatchList; mlist!=NULL;mlist=mlist->next )
            {
                printf("==================================================================Match KeyWord %s\n",mlist->patrn);
		strcpy(keyword,mlist->patrn);	


            }
        }

    return state;
}
/*
*   Free all memory
*/ 
void acsmfree (ACSMX_STRUCT * acsm) 
{
	
	int i;
	ACSMX_PATTERN * mlist, *ilist;
	for (i = 0; i <acsm->acsmNumStates; i++)

	{
		if (acsm->acsmStateTable[i].MatchList != NULL)

		{
			mlist = acsm->acsmStateTable[i].MatchList;
			while (mlist)
			{
				ilist = mlist;
				mlist = mlist->next;
				AC_FREE (ilist);
			}
		}
	}
	
	AC_FREE (acsm->acsmStateTable);

}

int scanAC(ACSMX_STRUCT * acsm, int state, char b, int* status,char* keyword)
{
    int isMatch=0;
    state=fsm(acsm,state,b,&isMatch,keyword);
    if(isMatch){
     *status=0;
    }
    else{
        if(acsm->dept[state]>=CDepth)
	     *status=1;
        else *status=2;
    }
    return state;
}
int  acch(ACSMX_STRUCT* acsm,unsigned char l[0x8000],unsigned short dbuf[0x8000],int offset,int last_lit,char* keyword,int* text_len)
{
        int n=last_lit;
        int tag=0;
        int w=0;
        int i;
        int status;
        int state=0;
       // char win[WSIZE];
       // int st[WSIZE];
        for(i=offset;i<n;i++)
        {
            if(dbuf[i]==0)
            {
                state=scanAC(acsm,state,l[i],&status,keyword);
                win[w]=l[i];
            if(status==0){
               // printf("========macth in 1111111============\n");
                tag=1;
		*text_len = i;
               // printf("breaking out of 111111111\n");
                break;
           }	
		if(w>=WSIZE-1){
		    printf("\nwsize large!\n");
		    return 0;}
                st[w++]=status;
            }
            else
            {
                int d=dbuf[i];
                int len=l[i]+3;
                int j=0;
                while(acsm->dept[state]>j&&j<len)
                {
                    if((w-d)>=0){
                        state=scanAC(acsm,state,win[w-d],&status,keyword);

                        win[w]=win[w-d];
			
			if(w>=WSIZE-1){
			    printf("\n wsize large !\n");
			    return 0;}
                        st[w++]=status;
                        j++;
		    if(status==0)
                        {
                            //printf("match in 22222222222222222\n");
                            tag=1;
                            //printf("breaking our of 2222222222222\n");
			    *text_len = i;
                            break;
                        }
                    }
                    else j++;
                }
                if(tag)
                {
                    //printf("breaking out of for\n");
                    break;
                }
                int k=j-1;
                int fk;
                int fp;
                int p;
                while(k<len-1)
                    {
                    k=len-1;
                    for(fk=j;fk<len;fk++)
		    {
                        if((w-d+fk)>=0)
                        {	
			    if(st[w-d+fk]==0)
			    {
                                k=fk;
                                break;
                            }
			}
                        else
                            ;
                    }
                    p=j;
                    for(fp=k;fp>=j;fp--)
                    {
                        if((w-d+fp)>=0&&st[w-d+fp]==2)
                        {
                            p=fp;
                            break;
                        }
                    }
                    if(j<(p-CDepth+1))
                    {
                        while(j<(p-CDepth+1))
                        {
                            win[w]=win[w-d];
                            w++;
                            j++;
                        }
                        state=0;
                        for(j=(p-CDepth+1);j<p;j++)
                        {
                            if((w-d)>=0)
                            {
                                state=scanAC(acsm,state,win[w-d],&status,keyword);
                                win[w]=win[w-d];
				if(w>=WSIZE-1){
				    printf("\n\nsize is large!\n\n");
				    return 0;}
                                st[w++]=status;
                                if(status==0)
                                {
                                    //printf("match in 3333333333333333333\n");
				    *text_len = i;
                                    tag=1;
                                    //printf("breaking out of 33333333333333\n");
                                    break;
                                }
                            }

                        }
                    }
                    int l;
                    for(l=j;l<=k;l++)
                    {
                    if((w-d)>=0)
                    {
                        state=scanAC(acsm,state,win[w-d],&status,keyword);

                        win[w]=win[w-d];
			
			if(w>=WSIZE-1){
			    printf("\n\n size is large\n\n"); 
			    return 0;}
                        st[w++]=status;
                        if(status==0)
                        {
                            //printf("match in 444444444444444444444444444\n");
                            tag=1;
			    *text_len = i;
                            //printf("breaking out of 4444444444444444444444\n");
                            break;
                        }
                    }
                }
                j=k+1;
if(tag)break;
            }
        }
        if(tag)break;
    }
if(tag){
    *text_len = i;
    return 1;
//printf("========================END OF ACCH FUNCTION WITH MATCHED==============================\n");
}
else{
    return 0;
//printf("========================END OF ACCH FUNCTION WITHOUT MATCHED==============================\n");
}
}
void replace(char* buf,int size);
int keyword_match(struct data_head* head,uch l[LIT_BUFSIZE],ush d[LIT_BUFSIZE],int len,int offset){

    int last_lit= 0;
    last_lit=len;
    char keyword[1024];
    if(M==NULL || N == NULL)
	return 0;
//    printf("len====%d\n",last_lit);
 //   printf("offset=====%d\n",offset);
/*    char* word[]={"tianchao","buaa"};
    int length = sizeof(word)/sizeof(char**);
    int i =0 ;
    ACSMX_STRUCT *F;
    F=acsmnew();
    for(;i <length;i++)
        acsmaddPattern(F,(unsigned char*)word[i],strlen(word[i]));
    acsmcompile(F);*/
    int result = 0;
    offset = 0;
    int text_length = 0;
    result = acch(M,l,d,offset,last_lit,keyword,&text_length);
    //

    int lop = 0;
    //printf("utos_length == %d\n",utos_length);	
    for(lop;lop < utos_length;lop++)
    {
	if(strstr(keyword,utos[lop].code) != NULL)
	{
		memset(keyword,0,strlen(keyword));
		memcpy(keyword,utos[lop].str,strlen(utos[lop].str));
		printf("keyword=======%s\n",keyword); 
	}
    }
    //

    if(result){
        char fulltext[0x80000];
	int log_len = 0;
	text_length = (text_length + 10) > last_lit ? last_lit:(text_length + 10);
	decodeing(l,d,text_length,fulltext,&log_len);
	if(log_len > 200)
		log_len -= 200;
	add_log(keyword,head,fulltext + log_len );//+ ((log_len-50) > 0?(log_len - 50):log_len));
        
     }
    return (result);
}
void add_log(char* keyword,struct data_head* head,char* fulltext)
{
	char test[1000];
	int size = 0;
	struct location_tuple* location = (struct location_tuple *) malloc(sizeof(struct location_tuple));
	char* str = "NULL";
	location->serverport = head->a_tcp->addr.dest;
	location->serverip = head->a_tcp->addr.daddr;
	location->clientport = head->a_tcp->addr.source;
	location->clientip = head->a_tcp->addr.saddr;
	struct loginfo log;
	initlog(&log,location);
	log_set_gk_type(&log,0);
	log_addkeyword(&log,keyword);
	/*printf("%s",fulltext);
	printf("\n\n*****************************\n\n");*/
        //log_set_full_text(&log, fulltext);
        int len_of_fulltext = 0;
        len_of_fulltext = strlen(fulltext);
	//printf("%d\n",len_of_fulltext);
	//printf("汉字占用字节大小为%d\n",strlen("中国"));
	if( 400 > len_of_fulltext)
		size = len_of_fulltext;
	else
		size = 400;
	//printf("\n\n");
	/*int t = 0;
	for(t=0;t<strlen(keyword);t++)
		printf("%x\t",keyword[t]&0xff);
	printf("\n\n");*/
        memcpy(test,fulltext,size);
	replace(test,size);
	//printf("%s",test);
	//test[size-1] = '\0';
	//printf("\n\n*******************************\n\n");
        //memset(log.fulltext, 0, 1000*sizeof(char));
        /*int i = 0;
	for(;i<10;i++)
		test[size+i] = ' ';*/
	int i = 0;
	while(i<size){
		unsigned int result = test[i]&0xff&0x80;
		if(result == 0)
			break;
		result = test[i]&0xff&0xe0;
		if(result== 224)
			break;
		result = test[i]&0xff&0x80;
		if(result== 128)
			i++;
	}
	int j = size -1 ;
	while(j>0){
		unsigned int score = test[j]&0xff&0x80;	
		if(score == 0)
			break;
		score = test[j]&0xff&0xe0;	
		if(score == 224){
			//j--;
			break;
		}
		score = test[j]&0xff&0x80;	
		if(score == 128)
			j--;
	}
	test[j] = '\0';
	printf("%s\n",test);
	//printf("i==%d\tj====%d\n",i,j);
	memcpy(log.fulltext,test+i,j);
	sendlog(&log,0);
	free(location);
}

void decodeing(unsigned char* litbuf,unsigned short *distbuf,int length,char* result, int* th)
{
	int index = 0;
	//char result[0x800000];
        int  i = 0;
	for(index;index < length;index++)
	{
		if(distbuf[index] == 0)
			result[i++] = litbuf[index];
		else{
			int dist = distbuf[index];
			int len = litbuf[index] + 3;
			int j = 0 ;
			for(j;j < len; j++)
				result[i++] =  result[i - dist];
 
        		}
	}
	result[i] = '\0';
        *th = i;
}
void replace(char* buf,int size)
{
	int i = 0;
	while(i < size)//buf[i] != '\0')
	{
		if(buf[i] == '\n' || buf[i] == '\r' || buf[i] == '|')
			buf[i] = ' ';
		i++;
	}	
	buf[i] = '\0';
}
