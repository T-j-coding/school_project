#include "http_parse.h"
#include <time.h>

static time_t start, end;
static int t = 0;
int _header_field_type(const char *at)
{
	if(!strncmp(at, "Cookie", 6))
		return COOKIE;
	if(!strncmp(at, "Host", 4))
		return HOST;
	if(!strncmp(at, "Content-Length", 14))
		return CONTENT_LEN;
	if(!strncmp(at, "Referer", 7))
		return REFERER;
	return 0;
}

int urldecode_my(char *p)  
{  
    int i=0;  
    while(*(p+i))  
    {  
       if ((*p=*(p+i)) == '%')  
       {  
        *p=*(p+i+1) >= 'A' ? ((*(p+i+1) & 0XDF) - 'A') + 10 : (*(p+i+1) - '0');  
        *p=(*p) * 16;  
        *p+=*(p+i+2) >= 'A' ? ((*(p+i+2) & 0XDF) - 'A') + 10 : (*(p+i+2) - '0');  
        i+=2;  
       }  
       else if (*(p+i)=='+')  
       {  
        *p=' ';  
       }  
       p++;  
    }  
    *p='\0'; 
	return 0;
}  

void _init_c_info(struct connection_info * c_info)
{
	bzero(c_info, sizeof(struct connection_info));
	c_info->user_id[0] = '\0';
	c_info->s_id[0] = '\0';
	c_info->r_id[0] = '\0';	
	c_info->comment[0] = '\0'; 
	c_info->p_type = 0;
}

int _page_type_(char *path)
{
     //   if(strstr(path,"e7f.jpg"))printf("from page_type:%s\n",path);
	if(strstr(path,"like")&&!strstr(path,"showlike"))//done!
		return LIKE;

	if(strstr(path, "blog/")) //&& !strstr(path,"blogs/")) //20150429,yangbr
		return NOTE;

	if(strstr(path, "NewEntry.do"))//done!
		return EDIT_NOTE;

	if(strstr(path, "album-create-new"))//done!
		return MEDIA_SET;

	//if(strstr(path, "photo")&&!strstr(path, "album-creat-new")&&!strstr(path,"save"))//done!
	if(strstr(path, "upload/"))
	    //&&(strstr(path,"nphoto")||strstr(path,"ajaxproxy")) )//20150429,yangbr
		return UPLOAD_PHOTO;

	//if(strstr(path,"ebpn/show")||strstr(path,"ajaxmsy"))
	if(strstr(path,".jpg")) //&&strstr(path,"rrimg")||strstr(path,"rrfnm")||strstr(path,"xnimg")||strstr(path,"xnpic")||strstr(path,"photo"))//20150429,yangbr
		return PHOTO;

	if(strstr(path,"addfollow")||strstr(path, "friend.do"))//done!
		return ADD_FRIEND;

	if(strstr(path,"reply")||strstr(path, "comment")||strstr(path,"Comment"))//done!
		return COMMENT;

	if(strstr(path, "status"))//done!
	{
	return STATUS;}

	if(strstr(path,"share/popup.do")||strstr(path,"share/submit.do")||strstr(path,"update.do")||strstr(path,"share/ajax.do"))//hang on
        {
                printf("IT IS A SHSARE:URL=%s\n",path);
		return SHARE;
        }

	if(strstr(path,"muc_chat"))//done
		return CHAT;
	return 0;
}

int _url_parse(struct http_parser*_, char *url,struct connection_info * c_info)
{
	char *path;
	char* temp=(char*)(url);
	URL *storage=&(_->gh->storage);
	parseURL(url,storage);
	(*storage).path.start += 1;
	if(storage->path.start == storage->path.end)
		return 0;
	path = readURLField(url, storage->path);
	if(c_info->p_type == 0){
		c_info->p_type = _page_type_(url);
                if(strstr(url,"rrimg"))printf("%s\n%d\n",url,c_info->p_type);
        }
	if(c_info->p_type == NOTE)
	{
	    if(strstr(path,"blogs/"))
		c_info->p_type=0;

	    char* s_start = strstr(path,"blog/");
	    if( NULL != s_start)
	    {
		memcpy(c_info->s_id,s_start+5 ,9);
		c_info->s_id[9] = '\0';
		char* r_start = strstr(s_start+6,"/");
		if(NULL!=r_start)
		{
		    memcpy(c_info->r_id, r_start+1, 9);
		    c_info->r_id[9] = '\0';
		}
	    }
	}
	else if(c_info->p_type == LIKE)
	{
		char* start=strstr(temp,"_");
		if(start)
		{
			int len=strchr(start+1,'&')-start-1;
			if(len>0&&len<52)
			{
			    memcpy(c_info->r_id,start+1,len);
			    c_info->r_id[len]='\0';
			}
			else
			    c_info->r_id[0]='\0';
		}
		char * start1=strstr(temp,"owner");
		if(start1)
		{
			int len1=strchr(start1+5,'&')-start1-5;
			if(len1>0&&len1<20)
			{
			    memcpy(c_info->s_id,start1+5,len1);
			    c_info->s_id[len1]='\0';
			}
			else
			    c_info->s_id[0]='\0';
		}
	}
	else if(c_info->p_type == UPLOAD_PHOTO)
	{
	    char* user_start = strstr(temp,"hostid=");
	    if(NULL!=user_start)
	    {
		memcpy(c_info->user_id, user_start+7, 9);
		c_info->user_id[9]='\0';
	    }
	}
	else if(c_info->p_type == PHOTO)
	{
	//TODO
	}
	return 0;
}

int on_url(http_parser* _, const char* at, size_t length) {
	struct connection_info *c_info=_->c_info;	
	(void)_;
	if((int)length > 300 || (int)length <2) 
		return -1;
	memcpy(_->gh->http.url, at, (int)length);
	_->gh->http.url[(int)length] = '\0';
	char* GET_test=_->GET_test;
	memset(GET_test, 0, 512 * sizeof(char));
	const char* suffix = "HTTP/1.1";
	char* ptr;
	if ((ptr = strstr(at, suffix)) != NULL) {
	    int i = 0;
	    for (i = 0; i < ptr - at - 1; i++)
		GET_test[i] = *(at + i);
	}
	_url_parse(_,_->gh->http.url, c_info);
	return 0;
}

int on_header_field(http_parser* _, const char* at, size_t length) {
	(void)_;
	_->gh->http_field_type =  _header_field_type(at);
	return 0;
}

int on_header_value(http_parser* _, const char* at, size_t length) {
	char*host_zhong;
	(void)_;
	struct connection_info *c_info=_->c_info;
	switch(_->gh->http_field_type)
	{
		case HOST:
		{
		    host_zhong=(char*)malloc((int)length+1);
		    memcpy(host_zhong,at,(int)length);
		    host_zhong[(int)length]='\0';
	
		    memset(c_info->url,0,512);
		    char* first = "http://";
		    strcat(c_info->url,first);
		    strcat(c_info->url,host_zhong);
		    strcat(c_info->url,_->GET_test);
		    free(host_zhong);
		    break;
		}
		case COOKIE: //unknow size of cookie, stay available
 			if(c_info->user_id[0]=='\0')
 			{
 				char *start = strstr(at, "; id=");
				if(start)
				{   
				    if(strchr(start+5, ';'))
				    {
					int len = strchr(start+5, ';')-start-5;
					if(len>0&&len<20)
					{
					    memcpy(c_info->user_id, start+5, len);
					    c_info->user_id[len] = '\0';
					}
					else
					    c_info->user_id[0]='\0';
				    }
				}
			}
			break;
		case CONTENT_LEN:
			_->gh->content_length = atoi(at);
			break;
		case REFERER:
			if(c_info->p_type==0){
				char *url = (char *)malloc((int)length+1);
				memcpy(url, at, (int)length);
				url[(int)length] = '\0';
				_url_parse(_,url, c_info);
				FREE(url);
			}
			break;
	}
	return 0;
}

int on_body(http_parser* _, const char* at, size_t length) {
	struct connection_info *c_info=_->c_info;
	(void)_;
	_->gh->con_len = (int)length;
	if(c_info->user_id[0] == '\0')qs_scanvalue("__user", at, c_info->user_id, sizeof(c_info->user_id));
	char* temp=(char*)(at);
        char* title;
        char* name;
        char* start4;
        char* start5; 
	switch(c_info->p_type){
		case COMMENT:
			qs_scanvalue("c", at, c_info->comment, sizeof(c_info->comment));
			qs_scanvalue("content", at, c_info->comment, sizeof(c_info->comment));
			qs_scanvalue("comment", at, c_info->comment, sizeof(c_info->comment));
			qs_scanvalue("body", at, c_info->comment, sizeof(c_info->comment));
			qs_scanvalue("entryId", at, c_info->r_id, sizeof(c_info->r_id));
			qs_scanvalue("id", at, c_info->r_id, sizeof(c_info->r_id));
			qs_scanvalue("source", at, c_info->r_id, sizeof(c_info->r_id));
			qs_scanvalue("entryOwnerId", at, c_info->s_id, sizeof(c_info->s_id));
			qs_scanvalue("owner", at, c_info->s_id, sizeof(c_info->s_id));
			break;
		case ADD_FRIEND:
			qs_scanvalue("id", at, c_info->s_id, sizeof(c_info->s_id));
			qs_scanvalue("pubId", at, c_info->r_id, sizeof(c_info->r_id));
			qs_scanvalue("why", at, c_info->comment, sizeof(c_info->comment));
			break;
		case EDIT_NOTE:
			title = (char *)malloc(100);
			qs_scanvalue("title", at, title, 100);
			qs_scanvalue("body", at, c_info->comment, sizeof(c_info->comment));
			strcat(c_info->comment, title);
			free(title);
			break;
		case STATUS:
                        // sunpy: for debug
			if(!strncmp(at,"channel",7)) { qs_scanvalue("content", at, c_info->comment, sizeof(c_info->comment));}
                        else { 
                            qs_scanvalue("content", at, c_info->comment, sizeof(c_info->comment));
                        }

			break;
		case MEDIA_SET:
			name = (char *)malloc(30);
			qs_scanvalue("name", at, name, 30);
			qs_scanvalue("description", at, c_info->comment, sizeof(c_info->comment));
			strcat(c_info->comment, name);
			free(name);
			break;
		case UPLOAD_PHOTO:
			qs_scanvalue("title", at, c_info->comment, sizeof(c_info->comment));
			break;
		case PHOTO:
			//qs_scanvalue("r",at,temp_url,sizeof(temp_url));
			break;
		case CHAT:
	                start4=strstr(temp,"<font>");
			if(start4)
			{
			    if(NULL!=strchr(start4+6,'<'))
			    {
				int len4=strchr(start4+6,'<')-start4-6;
				if(len4>0&&len4<1000)
				{
				    memcpy(c_info->comment,start4+6,len4);
				    c_info->comment[len4]='\0';
				}
				else
				    c_info->comment[0]='\0';
			    }
			}
	                start5=strstr(temp,"to=");
			if(start5)
			{
			    if(NULL!=strchr(start5+4,'@'))
			    {
				int len5=strchr(start5+4,'@')-start5-4;
				if(len5>0&&len5<20)
				{
				    memcpy(c_info->s_id,start5+4,len5);
				    c_info->s_id[len5]='\0';
				}
				else
				    c_info->s_id[0]='\0';
			    }
			}
			break;
		case SHARE:
		{
			if(!strncmp(at,"c=",2)){
                        qs_scanvalue("c",at,c_info->comment,sizeof(c_info->comment));
                        qs_scanvalue("fwdId",at,c_info->r_id,sizeof(c_info->r_id));
                        qs_scanvalue("fwdOwner",at,c_info->s_id,sizeof(c_info->s_id));
                        break;
                        }
                        char *temp_temp=(char*)malloc(100000*sizeof(char));
			memset(temp_temp,0,sizeof(char)*100000);
			qs_scanvalue("post",at,temp_temp,100000*sizeof(char));
                        printf("DECODE BODY=%s\n",temp_temp);
			char*p=strstr(temp_temp,"body\":\"");
			//char*q=strstr(temp_temp,"\",\"hText");
			char*q;
			if(p!=NULL){
                            p+=7;
                            q=strstr(p,"\",\"");
                            if(q!=NULL){
			    strncpy(c_info->comment,p,q-p);	
			    c_info->comment[q-p]='\0';
                            }
                            else 
                                c_info->comment[0]='\0';
                        }
                        else c_info->comment[0]='\0';
                       /* char* link_start=strstr(temp_temp,"link\":\"")+strlen("link\":\"");
                        char* link_end=strstr(temp_temp,"\",\"action");
                        if(link_start!=NULL&&link_end!=NULL){
                            strncpy(c_info->url,link_start,link_end-link_start);
                            c_info->url[link_end-link_start]='\0';
                            FREE(temp_temp);
                            break;
                        }*/

                        // sunpy: for parse BUG
                        char* resource_id=strstr(temp_temp,"\"id\":");
                        if(resource_id==NULL){FREE(temp_temp);break;}
                        resource_id+=strlen("\"id\":");
                        char* resource_id_end;
                        if(!strncmp(resource_id,"\"",1)){resource_id++;resource_id_end=strstr(temp_temp,"\",\"owner");}
                        else resource_id_end=strstr(temp_temp,",\"owner");
                        int r_id_len=resource_id_end-resource_id;
                        if(resource_id!=NULL&&resource_id_end!=NULL&&r_id_len>0&&r_id_len<=20){
                            strncpy(c_info->r_id,resource_id,r_id_len);
                            c_info->r_id[r_id_len]='\0';
                            // sunpy for debug
                            //printf("share id = %s\n", c_info->r_id);
                        } else {
                            //printf("share id is NULL\n");
                        }
			FREE(temp_temp);
			break;
                }
	}
	return 0;
}
char* fileRead(char *filename, long* file_length)
{
	FILE* file = fopen(filename, "r");
	if (file == NULL) {
	  perror("fopen");
	}

	fseek(file, 0, SEEK_END);
	*file_length = ftell(file);
	if (*file_length == -1) {
	  perror("ftell");
	}
	fseek(file, 0, SEEK_SET);
	char* data = malloc(*file_length);
	if (fread(data, 1, *file_length, file) != (size_t)*file_length) {
	  fprintf(stderr, "couldn't read entire file\n");
	  FREE(data);
	}
	return data;
}


struct connection_info * processhttp(char* data, int http_length)
{
	int i;
	char* p = data;
	struct http_parser_settings settings;
	size_t nparsed;
	memset(&settings, 0, sizeof(settings));
	settings.on_url = on_url;
	settings.on_header_field = on_header_field;
	settings.on_header_value = on_header_value;
	settings.on_body = on_body;
	http_parser parser;
	http_parser_init(&parser, HTTP_REQUEST);
        _init_c_info(parser.c_info);
	parser.GET_test= (char*)malloc(512 * sizeof(char));
	parser.gh=(struct global_http*)malloc(sizeof(struct global_http));
	parser.gh->nlines=0;
	parser.gh->isGzip=0;
	parser.gh->http_field_type=0;
	parser.gh->con_len=0;
	parser.gh->content_length=0;
	nparsed = http_parser_execute(&parser, &settings, data, (size_t)http_length);
	parser.gh->http.method = parser.method;
	/*if(parser.c_info->p_type==STATUS && parser.c_info->comment[0]=='\0')
	    parser.c_info->p_type = 0;
	if(parser.c_info->p_type==COMMENT && parser.c_info->comment[0]=='\0')
	    parser.c_info->p_type = 0;
	if(parser.c_info->p_type==LIKE && parser.c_info->s_id[0]=='\0' && parser.c_info->r_id[0]=='\0')
	    parser.c_info->p_type = 0;
        */
        /*
	if(parser.c_info->p_type == SHARE)
	{
	    char* aaa = (char*)malloc((1+http_length)*sizeof(char));
	    memset(aaa,0,(1+http_length)*sizeof(char));
	    memcpy(aaa,data,http_length);
	    free(aaa);
	}
        */

	/*
	if (nparsed != (size_t)http_length) 
	    printf( "Error: %s (%s)\n",http_errno_description(HTTP_PARSER_ERRNO(&parser)),http_errno_name(HTTP_PARSER_ERRNO(&parser)));
	*/

	if (parser.GET_test != NULL)
	    free(parser.GET_test);
	if(parser.gh->content_length !=  parser.gh->con_len && parser.gh->http.method == 3 && http_length < 4096)
	{
	    FREE(parser.gh);
	    return NULL;
	}
	else
	       FREE(parser.gh);
	return parser.c_info;
}
