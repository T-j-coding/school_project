#include "loop.h"
#include "assert.h"
#include <sys/time.h>


extern ACSMX_STRUCT* M;
extern ACSMX_STRUCT* N;
sqlite3 *db = NULL;
int init_sqlite()
{
    char* dbpath="shutter2.db";
    char *zErrMsg = 0;
    int rc;
    //open the database file.If the file is not exist,it will create a file.
    rc = sqlite3_open(dbpath, &db);
    if( rc )
    {
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return 0;
    }
    return 1;
}

int check_gk(struct data_head * dh)
{
    int ret = 0;
    
    if(pthread_mutex_trylock(&(dh->tcp_stream_lock_up))==0)
    {
        //has no tcp stream , or blocked already
        if(NULL == dh || NULL == dh->a_tcp || dh->a_tcp->gk_state==gk_method)
            ret = 1;
	else
	{
            if (NULL == dh->first_up)
		//printf("NO DATA SEGMENT\n");
		ret = 1;
	    else
	    {
		if(NULL == dh->temp_up)
		    dh->temp_up = dh->first_up;
		else
		{
		    if (dh->temp_up == dh->last_up)
			ret = 1;
		}
	    }
	}

        pthread_mutex_unlock(&(dh->tcp_stream_lock_up));
    }
    return ret;
}

void set_gk(struct data_head * dh, int method)
{
    //method gk_method
    if (method != 5)
        if(pthread_mutex_trylock(&(dh->tcp_stream_lock_up))==0)
        {
	    if (NULL != dh->a_tcp)
            {
                dh->a_tcp->gk_state = method;
               // sunpy: tmp
               //printf("### set_gk ###\n");
              // dh->a_tcp->gk_state =gk_method;
            }
            pthread_mutex_unlock(&(dh->tcp_stream_lock_up));
        }
}

void update_dh(struct data_head * dh, char ** request, int request_num)
{
    int len = 0;
    int sum = 0;
    struct data_segment* pds_temp;
    int i;
    for (i = 0; i < request_num; i++)
        len += strlen(request[i]);

    if(pthread_mutex_trylock(&(dh->tcp_stream_lock_up))==0)
    {
	if (NULL == dh->a_tcp)
        {
            pthread_mutex_unlock(&(dh->tcp_stream_lock_up));
            return;
        }
        //pds_temp = dh->first_up;
        //wunn add
        pds_temp = dh->temp_up;
        sum = 0;
        while (NULL != pds_temp && len > sum)
        {
            sum += pds_temp->len;
            pds_temp = pds_temp->next;
        }
        dh->temp_up = (NULL == pds_temp) ? dh->last_up : pds_temp;
        //dh->temp_up = dh->last_up;
        dh->offset_up = 0;
        pthread_mutex_unlock(&(dh->tcp_stream_lock_up));
    }
}

void init_result(struct decision_result*dr)
{
    dr->url=0;
    dr->gk=0;
    dr->monitor=0;
    dr->trie=0;
}

int get_decision(struct connection_info *c_info, struct location_tuple * lt)
//TODO int get_decision(struct data_head * dh,struct connection_info *c_info)
{
    struct decision_result*dr=(struct decision_result*)malloc(4*sizeof(int));
    init_result(dr);
    if(c_info==NULL)
    {
        if (NULL != dr)
            free(dr);
        return -1;
    }
    if(c_info->p_type==13)
    dr->trie=url_match(c_info->url);
    else
    dr->url= url_match(c_info->url);
    struct logic_query_result*lr=logic_query_match(c_info->comment);
    struct logic_query_result*lr_N=logic_query_match_N(c_info->comment);
    dr->gk=lr->hit_count;
    dr->monitor=lr_N->hit_count;
    int pattern1=-1,pattern2=-1, rt=0;
    dr->trie+=trie_chk_rule(c_info->user_id, c_info->s_id, c_info->p_type, c_info->r_id, &pattern1,&pattern2);
    if (dr->url==0&&dr->gk==0&&dr->monitor==0&&dr->trie==0)
    {
        free(dr);
	free(lr);
	free(lr_N);
        return 0;
    }
    printf("===========================\n");
    printf("url:%d\ngk:%d\nmonitor%d\ntrie:%d\n",dr->url,dr->gk,dr->monitor,dr->trie) ;
    printf("===========================\n");
    if(dr->monitor>0)
    {
 	//sunpy
 	if(dr->url == 0 && dr->gk == 0 && dr->trie == 0) {
         printf("hit count: %d\n", dr->monitor);

         int i;
         for (i = 0; i < dr->monitor; i++) {
             struct loginfo log;
             initlog(&log, lt);
             log_addsocialinfo(&log, c_info);

             // sunpy: after discussing with Shao
             log_set_gk_type(&log, 4);
            
             int index = lr_N->hit_rule_list[i];
             struct query_expression* qe = query_rule_table_N->table[index];
	     log_addkeyword(&log, qe->full_text);
             log_set_full_text(&log, c_info->comment);
            
             printlog(&log);
             sendlog(&log, dr->monitor);
         }
         
         free(lr);
         free(lr_N);
         free(dr);

         return 0;
        }
    }
        if(dr->url>0)
        {
            printf("============================\n");
            printf("HIT URL\n");
            printf("============================\n");
            struct loginfo log;
            initlog(&log, lt);
            log_addsocialinfo(&log, c_info);
            log_set_gk_type(&log, 3);
            
            char * kw[dr->url];
            int z;
            for(z=0;z<dr->url;z++){
                kw[z]=(char*)malloc(256*sizeof(char));
                bzero(kw[z],256*sizeof(char));
            }
            get_keywords(c_info->url,kw);
            log_addkeywords(&log,kw,dr->url);
            log.keywords[strlen(log.keywords)-1]='\0';
            log_addkeyword(&log, c_info->url);
            log_set_full_text(&log, c_info->comment);
            printlog(&log);
            sendlog(&log,0);
            for (z=0; z < dr->url; z++)
            	FREE(kw[z]);
            //FREE(kw);
        }
        if(dr->gk>0)
        {
            printf("============================\n");
            printf("HIT GK KEYWORD\n");
            printf("============================\n");
            int i;
            for (i = 0; i < dr->gk; i++) {
                struct loginfo log;
                initlog(&log, lt);
                log_addsocialinfo(&log, c_info);
                log_set_gk_type(&log, 0);
                
                int index = lr->hit_rule_list[i];
                struct query_expression* qe = query_rule_table->table[index];
		log_addkeyword(&log, qe->full_text);
		log_set_full_text(&log, c_info->comment);
                
                //printlog(&log);
                sendlog(&log,0);
            }
        }
        if(dr->trie>0)
        {
            printf("============================\n");
            printf("HIT TRIE \n");
            printf("============================\n");
            struct loginfo log;
            initlog(&log, lt);
            log_addsocialinfo(&log, c_info);
            if (pattern1 != -1) {
                if (pattern1 % 2 == 0)
                    log_set_gk_type(&log, 1);
                else
                    log_set_gk_type(&log, 2);
            } else {
                log_set_gk_type(&log,2);
            }
            log_set_full_text(&log, c_info->comment);
//            printlog(&log);
            sendlog(&log,0);
        }
		
    free(lr);
    free(lr_N);
    free(dr);
	return 1;
}

char * get_buffer(struct data_head * dh, struct location_tuple * lt)
{
    int this_length = 0, len = 0;
    struct data_segment * ds = NULL;
    char * buffer = NULL;

    if(pthread_mutex_trylock(&(dh->tcp_stream_lock_up))==0)
    {
	if (NULL == dh->a_tcp)
        {
            pthread_mutex_unlock(&(dh->tcp_stream_lock_up));
            return buffer;
        }
		
        if (NULL != dh->temp_up)
        {
            this_length = dh->temp_up->len - dh->offset_up;
            ds = dh->temp_up->next;
            while (NULL != ds && ds != dh->last_up->next)
            {
                this_length += ds->len;
                ds = ds->next;
            }   //get length
        }
        else
            this_length = 0;

        buffer = (char *) malloc((1 + this_length) * sizeof(char));
        *(buffer + this_length) = '\0';
	len = this_length;

	//printf("%s", dh->temp_up->data);
	if (NULL != dh->temp_up)
        {
            this_length = dh->temp_up->len - dh->offset_up;
            struct data_segment* temp_up_backup = dh->temp_up;
            memcpy(buffer, dh->temp_up->data+dh->offset_up, this_length);
            dh->temp_up = temp_up_backup;
            ds = dh->temp_up->next;
            while (NULL != ds && ds != dh->last_up->next && this_length < len)
            {
	        //printf("buffer:%d %s\n", ds->len, ds->data);
                memcpy(buffer+this_length, ds->data, ds->len);
	        //printf("%s", ds->data);
                this_length += ds->len;
                ds = ds->next;
            }
	    //printf("\nbuffer:%s\n", buffer);
            //printf("len=%d length=%d\n", len, this_length);
        }

	lt->serverport = dh->a_tcp->addr.dest;
	lt->serverip  =  dh->a_tcp->addr.daddr;
	lt->clientport = dh->a_tcp->addr.source;
	lt->clientip  =  dh->a_tcp->addr.saddr;

        pthread_mutex_unlock(&(dh->tcp_stream_lock_up));
    }
    return buffer;
}
int get_ReN(char *buffer,regex_t reg,int *offset,int *len){
    int counter=0,anchor=0;
    regmatch_t pmatch[1];
    int so=0,eo=0,buffer_len=strlen(buffer);
    do{
        if(regexec(&reg,buffer,1,pmatch,0)==REG_NOMATCH)break;
        else{
            anchor+=(eo-so);
            so=pmatch[0].rm_so;eo=pmatch[0].rm_eo;
            anchor+=so;
            //if(so==0||*(buffer+so+2)=='\\')offset[counter++]=anchor;
            offset[counter++]+=anchor;
            buffer+=eo;
        }
    }while(eo!=strlen(buffer));
    int i=0;offset[counter]=buffer_len;
    while(i<counter){len[i]=offset[i+1]-offset[i];i++;}
    return counter;
}
char ** get_request(char *buffer, int *request_number)
{
    int i;
    char ** requests = NULL;
    int request[100],len[100];memset(len,0,sizeof(int)*100);memset(request,0,sizeof(int)*100);
    char * tmp = NULL;
    int length;
    char * p;
    *request_number = get_ReN(buffer,regForHttp,request,len);
    if (*request_number == 0)
	return NULL;
    requests = (char **) malloc(*request_number * sizeof(char *));
   // request = (int *) malloc((*request_number + 1) * sizeof(int));
    //*(request + *request_number) = strlen(buffer);
   // get_request_offset(buffer,request);

    // sunpy: for bug
    p = buffer + request[0];
    
    for (i = 0; i < *request_number; i++){
        length = len[i];
        tmp = (char *) malloc((length + 1) * sizeof(char));
        *(tmp + length) = '\0';
        memcpy(tmp, p, length);
        requests[i] = tmp;
        p = p + length;
    }
    return requests;
}

void process_data_head(struct data_head * dh, int method,int index)
{
    if (check_gk(dh))
        return;

    char * buffer = NULL;
    char ** request = NULL;
    int request_number=0;
    int isgk = 0;
    int i;

    //Wang Xuyang
    struct location_tuple *lt;
    lt = (struct location_tuple *) malloc(sizeof(struct location_tuple));
    if((buffer = get_buffer(dh, lt))!=NULL)
    	request = get_request(buffer, &request_number);

    //const char * pattern_nan = "album123456789";
    //const char * pattern_nan = "北京";
    const char * pattern_nan = "111e7f.jpg";

    //printf("===================loop=============\n");
    //printf("buffer=%s\n\n\npattern_nan=%s\n",buffer,pattern_nan); 
    //printf("===================loop=============\n");
    if (buffer != NULL && strstr(buffer, pattern_nan))
    {
        printf("\n\nget_buffer\n");
        printf("a_tcp:%d\n", dh->a_tcp);
        printf("buffer length=%d, get_buffer done\n",strlen(buffer));
        printf("buffer=\n%s\n",buffer);
        //wunn add
        for (i = 0; i < request_number; i++)
            printf("Req:%s\n", request[i]);
    }
   
    //Over

    for (i = 0; i < request_number; i++)
    {
        struct connection_info * c_info = NULL;
        //Zhong Yuan
        //printf("==================request %d =%s\n==================\n",i,request[i]);
        c_info = processhttp(request[i],strlen(request[i]));
        //if(c_info != NULL && c_info->p_type==PHOTO)printf("PHOTO URL=%s\n",c_info->url);
	//printf("url:%s\n", c_info->url);
        int status = get_decision(c_info, lt);
        //Over
      
	if (request[i] != NULL && strstr(request[i], pattern_nan))
        {
            //printf("\n\nRequest: %s\n", request[i]);
            if (c_info != NULL)
	    {
	    // sunpy
	    // for third party test
	        if (strlen(c_info->comment) >= 0) {
                printf("Thread Id =%d\n",index);
                printf("gk_status:%d\n", status);
                printf("c_info.userid:%s\n", c_info->user_id);
	        printf("c_info.s_id:%s\n", c_info->s_id);
	        printf("c_info.ptype:%d\n", c_info->p_type);
	        printf("c_info.r_id:%s\n", c_info->r_id);
	        printf("c_info.comment:%s\n", c_info->comment);
	        printf("c_info.url:%s\n", c_info->url);
	        printf("==================================\n\n");
		}
            }
	}

        if (status == 0 || status == -1)
        {
            if (c_info != NULL)
                free(c_info);
            continue;
        }

        //sent log

        set_gk(dh, method);
        isgk = 1;
        printf("Thread Id =%d\n",index);
        printf("\n\ngk_status:%d\n", status);
	printf("c_info.userid:%s\n", c_info->user_id);
	printf("c_info.s_id:%s\n", c_info->s_id);
	printf("c_info.ptype:%d\n", c_info->p_type);
	printf("c_info.r_id:%s\n", c_info->r_id);
	printf("c_info.comment:%s\n", c_info->comment);
	printf("c_info.url:%s\n", c_info->url);
        if (c_info != NULL)
            free(c_info);
	//printf("\n\n");

        break;
    }

    if (!isgk)
    {
        update_dh(dh, request, request_number);
	//printf("update_dh:%d\n", request_number);
    }
    if (lt != NULL)
        free(lt);
    if (buffer != NULL)
        free(buffer);
    for (i = 0; i < request_number; i++)
        if (NULL != request[i])
            free(request[i]);
    if (NULL != request)
        free(request);
}


void dump_all(struct data_head * dh)
{
    if(pthread_mutex_trylock(&(dh->tcp_stream_lock_up))==0)
    {
        if(NULL != dh->a_tcp){
            struct data_segment * ds;
            for(ds=dh->first_up;;ds=ds->next){
                int i=0;
                for(i=0;i<ds->len;++i){
                    printf("%c",ds->data[i]);
                }
                if(ds==dh->last_up) break;
            }
        }
	pthread_mutex_unlock(&(dh->tcp_stream_lock_up));
    }
}

void* loop(void *arg)
{
    printf("=============start loop up================\n");
    int loopcount = 0;

    int index = *((int *)arg);
    int steplen = DATA_CACHE_TABLE_SIZE/THREAD_NUM;
    int startidx = index*steplen;
    int lastidx = index == (THREAD_NUM-1) ? DATA_CACHE_TABLE_SIZE : (index+1)*steplen;

    struct timeval start, end;
    int count_test = 0;
    int sum_time = 0;
   
    loopcount = startidx;
    for(; loopcount<lastidx;)
    {
        //printf("%d--->",loopcount);
        //poll the linked-list in one data_cache_table, send each data_head to process_data_head()
        struct data_head * dh_pos = &data_cache_table[loopcount];
        while(dh_pos!=NULL)
        {
            //gettimeofday( &start, NULL );
            process_data_head(dh_pos, gk_method,loopcount);
            //gettimeofday( &end, NULL );
            //int timeuse = 1000000 * ( end.tv_sec - start.tv_sec ) + end.tv_usec - start.tv_usec;
           // if (count_test > 3000000)
           // {
                //printf("time: %6.2f us\n", sum_time*1.0/count_test);
             //   count_test = 0;
              //  sum_time = 0;
           // }
            //count_test++;
            //sum_time += timeuse;

            dh_pos = dh_pos->next;
        }
        //printf("\n");

        //following lines for endless loop
        if ( loopcount == lastidx-1 )
        {
            loopcount = startidx;
            continue;
        }
        loopcount++;
    }
}

void init_loop()
{
    init_sqlite();
    init_ac();
    init_trie_tree();
    init_query_table();
    init_query_table_N();
    down_ac();
    add_query_rule("Li & Hong # Zhi");
    add_query_rule("free");
    add_query_rule("sun # peiyuan");
}
