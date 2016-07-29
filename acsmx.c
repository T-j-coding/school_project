#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "acsmx.h"
#include <sys/time.h>

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
static void AddMatchListEntry (ACSM_STRUCT * acsm, int state, ACSM_PATTERN * px) 
{
	ACSM_PATTERN * p;
	p = (ACSM_PATTERN *) AC_MALLOC (sizeof (ACSM_PATTERN));
	memcpy (p, px, sizeof (ACSM_PATTERN));

	/*Add the new pattern to the pattern  list*/
	p->next = acsm->acsmStateTable[state].MatchList;
	acsm->acsmStateTable[state].MatchList = p;
}

/* 
* Add Pattern States
*/ 
static void AddPatternStates (ACSM_STRUCT * acsm, ACSM_PATTERN * p) 
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
		ACSM_STATETABLE state_p;
		state_p.MatchList=0;
        acsm->acsmStateTable.push_back(state_p);
	    for (i = 0; i < ALPHABET_SIZE; i++)
	    {
		    acsm->acsmStateTable[acsm->acsmNumStates].NextState[i] = ACSM_FAIL_STATE;
	    }*/
		acsm->acsmStateTable[state].NextState[*pattern] = acsm->acsmNumStates;
		state = acsm->acsmNumStates;
	}
	/*Here,An accept state,just add into the MatchListof the state*/
	AddMatchListEntry (acsm, state, p);
}


/*
*   Build Non-Deterministic Finite Automata
*/ 
static void Build_DFA (ACSM_STRUCT * acsm) 
{
	int r, s;
	int i;
	QUEUE q, *queue = &q;
	ACSM_PATTERN * mlist=0;
	ACSM_PATTERN * px=0;

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

                                
                                ACSM_PATTERN* pat = acsm->acsmStateTable[next].MatchList;
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
ACSM_STRUCT * acsmNew () 
{
	ACSM_STRUCT * p;
	p = (ACSM_STRUCT *) AC_MALLOC (sizeof (ACSM_STRUCT));
	if (p)
		memset (p, 0, sizeof (ACSM_STRUCT));
	return p;
}


/*
*   Add a pattern to the list of patterns for this state machine
*/ 
int acsmAddPattern (ACSM_STRUCT * p, unsigned char *pat, int n) 
{
	ACSM_PATTERN * plist;
	plist = (ACSM_PATTERN *) AC_MALLOC (sizeof (ACSM_PATTERN));
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



int acsmDelPattern (ACSM_STRUCT * p, char *pat, int n) 
{
	ACSM_PATTERN * plist;
	ACSM_PATTERN * culist;
	
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
int acsmCompile (ACSM_STRUCT * acsm) 
{
	int i, k;
	ACSM_PATTERN * plist;

	/* Count number of states */ 
	acsm->acsmMaxStates = 1; /*State 0*/
	for (plist = acsm->acsmPatterns; plist != NULL; plist = plist->next)
	{
		acsm->acsmMaxStates += plist->n;
	}

	acsm->acsmStateTable = (ACSM_STATETABLE *) AC_MALLOC (sizeof (ACSM_STATETABLE) * acsm->acsmMaxStates);
//	MEMASSERT (acsm->acsmStateTable, "acsmCompile");
	memset (acsm->acsmStateTable, 0,sizeof (ACSM_STATETABLE) * acsm->acsmMaxStates);

	/* Initialize state zero as a branch */ 
	acsm->acsmNumStates = 0;
/*	
//del by ybr, modify to c
	ACSM_STATETABLE state_p;
	state_p.MatchList=0;
    acsm->acsmStateTable.push_back(state_p);
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
	Build_DFA (acsm);
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
int acsmSearch (ACSM_STRUCT * acsm, unsigned char *Tx, int n) 
{
	
	
	int state;
	ACSM_PATTERN * mlist;
	unsigned char *Tend;
	
    ACSM_STATETABLE * StateTable = acsm->acsmStateTable;
	

	int nfound = 0; /*Number of the found(matched) patten string*/
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


/*
*   improvment of acsmSearch()
*   get all the keyword offsets into an array (modified by yangbr)
*/

int acsmSearch_offset (ACSM_STRUCT * acsm, unsigned char *Tx, int n, int offsetlist[] )
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
}

/*
 * *   another improvment of acsmSearch()
 * *   get all the keyword into an char* array (modified by yangbr)
 * */
int acsmSearch_kw (ACSM_STRUCT * acsm, unsigned char *Tx, int n, char* keywords[] )
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

        if( StateTable[state].MatchList != NULL )
        {

            for( mlist=StateTable[state].MatchList; mlist!=NULL;mlist=mlist->next )
            {
                index = T - mlist->n + 1 - Tx;

                memcpy(keywords[nfound], mlist->patrn, mlist->n);
                nfound++;
            }
        }
    }
    return nfound;
}


/*
*   Free all memory
*/ 
void acsmFree (ACSM_STRUCT * acsm) 
{
	
	int i;
	ACSM_PATTERN * mlist, *ilist;
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
/*
int main()
{
	ACSM_STRUCT * acsm;
    acsm = acsmNew();
	char str[100]="computer aslhf isdlajflka sklfdjsdkl jfc science alkhlsdf computer";
	//acsmAddPattern(acsm,"computer",strlen("computer"));
	//acsmAddPattern(acsm,"science",strlen("science"));
	acsmCompile(acsm);
	int n = acsmSearch(acsm,(unsigned char*)str,strlen(str));
	printf("find:%d\n",n);
	acsmFree(acsm);

	return 1;
}
*/
