#include "loop.h"
//#include "ungzip.h"
#include<time.h> 
sqlite3 *db = NULL;
//extern ACSMX_STRUCT* M;
//extern ACSMX_STRUCT* N;
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
        if(NULL == dh || NULL == dh->a_tcp || 
	    NULL == dh->a_tcp||dh->a_tcp->gk_state==gk_method)
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
                pthread_mutex_unlock(&(dh->tcp_stream_lock_up));
            }
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
        pds_temp = dh->first_up;
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

int get_decision(struct connection_info *c_info)
{
    struct decision_result*dr=(struct decision_result*)malloc(4*sizeof(int));
    init_result(dr);
    if(c_info==NULL)
    {
        if (NULL != dr)
            free(dr);
        return -1;
    }
    dr->url= url_match(c_info->url);
    struct logic_query_result*lr=logic_query_match(c_info->comment);
    struct logic_query_result*lr_N=logic_query_match_N(c_info->comment);
    dr->gk=lr->hit_count;
    dr->monitor=lr_N->hit_count;
    free(lr);
    free(lr_N);
    int pattern1=-1,pattern2=-1, rt=0;
    dr->trie=trie_chk_rule(c_info->user_id, c_info->s_id, c_info->p_type, c_info->r_id, &pattern1,&pattern2);
    if (dr->url==0&&dr->gk==0&&dr->monitor==0&&dr->trie==0)
    {
        free(dr);
        return 0;
    }
    printf("===========================\n");
    printf("url:%d\ngk:%d\nmonitor%d\ntrie:%d\n",dr->url,dr->gk,dr->monitor,dr->trie) ;
    printf("===========================\n");
    if(dr->monitor>0)
    {
        if(dr->url==0&&dr->gk==0&&dr->trie==0)
        {
            printf("============================\n");
            printf("HIT MONITOR KEYWORD ONLY\n");
            printf("============================\n");
	    return 0;
        }
        else
        {
            printf("============================\n");
            printf("HIT MONITOR KEYWORD\n");
            printf("============================\n");
        }
     } 
        if(dr->url>0)
        {
            printf("============================\n");
            printf("HIT URL\n");
            printf("============================\n");
        }
        if(dr->gk>0)
        {
            printf("============================\n");
            printf("HIT GK KEYWORD\n");
            printf("============================\n");
        }
        if(dr->trie>0)
        {
            printf("============================\n");
            printf("HIT TRIE \n");
            printf("============================\n");
        }
	return 1;
}

char * get_buffer(struct data_head * dh)
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
            while (ds != dh->last_up->next)
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
            memcpy(buffer, dh->temp_up->data+dh->offset_up, this_length);
            ds = dh->temp_up->next;
            while (ds != dh->last_up->next && this_length < len)
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
        pthread_mutex_unlock(&(dh->tcp_stream_lock_up));
    }
    return buffer;
}

char ** get_request(char *buffer, int *request_number)
{
    int i;
    char ** requests = NULL;
    int * request = NULL;
    char * tmp = NULL;
    int length;
    char * p;
    *request_number = get_request_number(buffer);
    if (*request_number == 0)
	return NULL;
    requests = (char **) malloc(*request_number * sizeof(char *));
    request = (int *) malloc((*request_number + 1) * sizeof(int));
    *(request + *request_number) = strlen(buffer);
    get_request_offset(buffer,request);
    p = buffer;
    
    for (i = 0; i < *request_number; i++){
        length = request[i+1] - request[i];
        tmp = (char *) malloc((length + 1) * sizeof(char));
        *(tmp + length) = '\0';
        memcpy(tmp, p, length);
        requests[i] = tmp;
        p = p + length;
    }
    free(request);
    return requests;
}

void process_data_head(struct data_head * dh, int method)
{
    if (check_gk(dh))
        return;

    char * buffer = NULL;
    char ** request = NULL;
    int request_number=0;
    int isgk = 0;
    int i;

    //Wang Xuyang
    if((buffer = get_buffer(dh))!=NULL)
    	request = get_request(buffer, &request_number);

    //printf("get_buffer\n");
    //printf("buffer length=%d, get_buffer done\n",strlen(buffer));
    //printf("buffer=\n%s\n\n",buffer);
    //Over

    for (i = 0; i < request_number; i++)
    {
        struct connection_info * c_info = NULL;
        //Zhong Yuan
        c_info = processhttp(request[i],strlen(request[i]));
	//printf("url:%s\n", c_info->url);
        int status = get_decision(c_info);
        //Over
        
        if (c_info != NULL)
	{
	    // sunpy
	    // for third party test
	    if (strlen(c_info->comment) > 2) {
                printf("gk_status:%d\n", status);
	        printf("c_info.userid:%s\n", c_info->user_id);
	        printf("c_info.s_id:%s\n", c_info->s_id);
	        printf("c_info.ptype:%d\n", c_info->p_type);
	        printf("c_info.r_id:%s\n", c_info->r_id);
	        printf("c_info.comment:%s\n", c_info->comment);
	        printf("c_info.url:%s\n", c_info->url);
	        printf("\n\n");
	    }

            free(c_info);
	}

        if (status == 0 || status == -1)
            continue;

        //sent log

        set_gk(dh, method);
        isgk = 1;

        printf("\n\ngk_status:%d\n", status);
	printf("c_info.userid:%s\n", c_info->user_id);
	printf("c_info.s_id:%s\n", c_info->s_id);
	printf("c_info.ptype:%d\n", c_info->p_type);
	printf("c_info.r_id:%s\n", c_info->r_id);
	printf("c_info.comment:%s\n", c_info->comment);
	printf("c_info.url:%s\n", c_info->url);
	//printf("\n\n");

        break;
    }

    if (!isgk)
    {
        update_dh(dh, request, request_number);
	//printf("update_dh:%d\n", request_number);
    }

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
    
    loopcount = startidx;
    for(; loopcount<lastidx;)
    {
        //printf("%d--->",loopcount);
        //poll the linked-list in one data_cache_table, send each data_head to process_data_head()
        struct data_head * dh_pos = &data_cache_table[loopcount];
        while(dh_pos!=NULL)
        {
            process_data_head(dh_pos, gk_method);
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

    add_query_rule("Li & Hong # Zhi");
    add_query_rule("free");
    add_query_rule("sun # peiyuan");
}
