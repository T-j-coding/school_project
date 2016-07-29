//extern "C"{
#include "down_string_match.h"
extern ACSMX_STRUCT* N;
extern ACSMX_STRUCT* M;
//}
//#include "acsm.h"
char valueToHexCh(const int value)
{
	char result = '\0';
	if(value >= 0 && value <= 9)
	{
		result = (char)(value + 48);
	
	}
	else
	{
		result = (char)(value - 10 + 97);
	}
	return result;
}
int strtounicode(char* src,char* des)
{
	char* output = des;
	char* str = src;
	int i = 0;
	int j = 0;
	int length = strlen(src);
	for(i;i<length;)
	{
		unsigned int temp = str[i] & 0x80;
		if((temp == 0))
		{
			output [j++] = str[i];  
			i++;
		}
		else
		{
			output[j++] = '\\';
			output[j++] = 'u';
			char unicode[2];
			char b1,b2,b3;
			unsigned int high = 0;
			unsigned int low = 0;
			int temp = 0;
			b1 = str[i++];
			b2 = str[i++];
			b3 = str[i++];
			if((b2 & 0xC0) != 0x80 || ((b3 & 0xC0) != 0x80))
				return 1;
			*(unicode) = (b1 << 4) + ((b2 >> 2) & 0x0F);
			//temp = (int)*unicode;
			high = *unicode & 0xF0;
			high = high >> 4;
			low = *unicode & 0x0F;
			output[j++]= valueToHexCh(high);
			output[j++]= valueToHexCh(low);
			*(unicode + 1) = (b2 << 6) + (b3 & 0x3F);
			//temp = (int)*(unicode + 1);
			high = *(unicode + 1) & 0xF0;
			high = high >> 4;
			low = *(unicode + 1) & 0x0F;	
			output[j++]= valueToHexCh(high);
			output[j++]= valueToHexCh(low);
			//output[j++] = *(unicode +1);
		}
		
	}
	output[j++] = '\0';
	/*int m = 0;
	for(m;m<j;m++)
		printf("%c\t",output[m]);
	printf("\n");*/
	return 0;
			
}			
int add_str(const char *s)
{
	int  length = strlen(s);
        char* string = (char*)malloc(length+1);
        bzero(string, length+1);
        memcpy(string, s, length);
        acsmaddPattern(N, (unsigned char*)string, length);
        free(string);
        M->acsmPatterns=N->acsmPatterns;
    	acsmcompile(N);
    	ACSMX_STRUCT * E=M;
    	M=N;
    	N=E;

    return 1;

}

int del_str(const char* s)
{
	int length = strlen(s);
        char* string = (char*)malloc(length+1);
        bzero(string, length+1);
        memcpy(string, s, length);

        acsmdelPattern(N, string, length);
        free(string);

        M->acsmPatterns=N->acsmPatterns;
	acsmcompile(N);

    	ACSMX_STRUCT * E=M;
    	M=N;
    	N=E;

    return 1;

}
int add_key_from_db()
{
	char* sql = "select keyword from keywordtable";
        int nrow=0,ncolumn;
    	char *zErrMsg;
    	char **azResult;
    	sqlite3_get_table(db, sql, &azResult, &nrow, &ncolumn, &zErrMsg);
        if(ncolumn!=1 || nrow ==0)
        {
		//printf("sjfkafjakjfaskjsadklgj");
                return 0;
        }
	else{
	 int i;
                for(i=1; i<=nrow; i++)
                {
			//printf("%s\n",azResult[i]);	
                        //add_str(azResult[i]);
                }
		
		add_str("法轮功0s0s");
		//add_str("2387605198");
		//add_str("万万没想到");
		/*add_str("百花ABC");
		add_str("百花之夜");
		add_str("人人热点");
		add_str("从一女牛牛");
		add_str("北航");
		add_str("zhongxin");
		add_str("buaa");
		//add_str("新鲜事");
		add_str("迎接舞团小鲜肉");
		add_str("金鸡百花奖");
		add_str("百花奖");
		add_str("中国电影");
		add_str("范冰冰");
		add_str("tfboy");
		add_str("找大学同学");*/
		
        }
        return 1;
}
ACSMX_STRUCT*  down_ac()
{
	M = acsmnew();
	N = acsmnew();
	add_key_from_db();
//	acsmcompile(M);
	//if(M!=NULL)
	  //  printf("M==%d\n",&M);
	return M;
	
}

