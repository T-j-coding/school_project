#define  _GNU_SOURCE
#include "get_data.h"
#include "loop.h"
#include "rule_accepter.h"
#include "acsm.h"
#include "mysql_conn.h"

ACSMX_STRUCT* M = NULL;
ACSMX_STRUCT* N = NULL;
int cpu_bind()
{
    int ret;
    int cpu_num=sysconf(_SC_NPROCESSORS_CONF);
    printf("cpu num: %d\n",cpu_num);

    cpu_set_t mask;
    int i=0;
    for(i=0;i<pkt_queue_num;++i){
        CPU_ZERO(&mask);
        CPU_SET(i%6+(i/6)*12,&mask);
        if (pthread_setaffinity_np(threads[i], sizeof(mask), &mask) < 0) {
            fprintf(stderr, "set thread affinity failed\n");
            return -1;
        }
    }
    CPU_ZERO(&mask);
    CPU_SET(i%6+(i/6)*12,&mask);
    if (pthread_setaffinity_np(pcap_thread, sizeof(mask), &mask) < 0) {
        fprintf(stderr, "set thread affinity failed\n");
        return -1;
    }
    return ret;
}
pthread_t pid_loop_thread[THREAD_NUM];

int create_loop_thread()
{
    int status;
    int i=0;
    int* args = malloc(THREAD_NUM*sizeof(int));
    for(i;i<THREAD_NUM;i++)
    {
	args[i] = i;
    	status = pthread_create( &pid_loop_thread[i], NULL, loop, args+i);
    	if ( status != 0 )
    	{
            printf( "pthread_create( pid_loop_thread ) fail.\n");
            //TODO:log
            goto end;
    	}
    }
    return 0;
end:
    return -1;
}

pthread_t pid_rule_accepter_thread[3];
int create_rule_accepter_thread()
{
    int status;
    status = pthread_create( &pid_rule_accepter_thread, NULL, startlisten, NULL );
    if ( status != 0 )
    {
	printf( "pthread_create( pid_rule_accepter_thread ) fail.\n");
	//TODO:log
	goto end;
    }
    return 0;
end:
    return -1;
}


// sunpy & wunannan
pthread_t pid_rule_timer_thread;
int create_rule_timer_thread()
{
    int status;

    struct database_waterline* waterline = load_all_config();

    status = pthread_create(&pid_rule_timer_thread, NULL, load_config, waterline);
    if (status != 0)
    {
        printf("pthread_create: rule timer failed.\n");
        return -1;
    }
    pthread_join(pid_rule_timer_thread, NULL);

    return 0;
}


int main ()
{
    system_init();

    create_rule_accepter_thread();

    init_loop();
    create_loop_thread();
    system_run();
    cpu_bind();

    create_rule_timer_thread();

    pthread_join(pcap_thread,NULL);
    pthread_join(pid_rule_accepter_thread, NULL);
    pthread_join(pid_up_stream, NULL);


    /*int i = 0;
    for(;i<3;i++)
       pthread_join(pid_loop_thread[i], NULL);*/
    return 0;
}
