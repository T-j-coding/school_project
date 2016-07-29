#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <netinet/ip.h>
#include <sys/time.h>
#include <net/ethernet.h>
#include <sched.h>
#include <pthread.h>
#include <unistd.h>
//#include<sys/types.h>
#include <sys/sysinfo.h>

#include <pcap.h>
//#include <libnet.h>
#include "packet_send.h"

char *dev_a, *dev_b;

pcap_t *pt_a, *pt_b;

pthread_t pid_a, pid_b;
//libnet_t *net_a, *net_b;
struct packet_send_handler net_a,net_b;

static char errbuf[256];
/*
void sigproc( int sig );
void* dev_a_capture( void *);
void dev_a_handle( u_char *devId, const struct pcap_pkthdr *h, const u_char *p );

void* dev_b_capture( void *);
void dev_b_handle( u_char *devId, const struct pcap_pkthdr *h, const u_char *p );
*/

/* single processing function */
void sigproc( int sig )
{
    pthread_cancel( pid_a );
    pthread_cancel( pid_b );
    pcap_close( pt_a );
    pcap_close( pt_b );
    //libnet_destroy( net_a );
    //libnet_destroy( net_b );
    packet_send_close(&net_a);
    packet_send_close(&net_b);

    printf("exit transmit. ");
    exit(0);
}

void dev_a_handle( u_char *devId, const struct pcap_pkthdr *hdr, const u_char *packet )
{
    //printf("%s,capture size :%d  ",devId, hdr->caplen );

    struct ether_header ehdr;
    memcpy( &ehdr, packet, sizeof( struct ether_header ));

    //printf("DST: %X:%X:%X:%X:%X:%X\n",ehdr.ether_dhost[0],ehdr.ether_dhost[1],
            //ehdr.ether_dhost[2],ehdr.ether_dhost[3],ehdr.ether_dhost[4],ehdr.ether_dhost[5]);
    if( ehdr.ether_shost[ETH_ALEN-1] == 0x10 )
    {
        return;
    }
    if( ehdr.ether_shost[ETH_ALEN-1] == 0xce )
    {
       //printf("TEST  src: %X:%X:%X:%X:%X:%X, dst:%X\n", ehdr.ether_shost[0],ehdr.ether_shost[1],
           //ehdr.ether_shost[2],ehdr.ether_shost[3],ehdr.ether_shost[4],
           //ehdr.ether_shost[ETH_ALEN-1], ehdr.ether_dhost[ETH_ALEN-1] );
        return;
    }
    if( ehdr.ether_dhost[ETH_ALEN-1] == 0xce || ehdr.ether_dhost[ETH_ALEN-1] == 0xff )
    {
        //if(ehdr.ether_dhost[ETH_ALEN-1] == 0xce)
            //printf("A  src:%X, dst:%X \n", ehdr.ether_shost[ETH_ALEN-1], ehdr.ether_dhost[ETH_ALEN-1] );
        int c;
        //while(( c = packet_send_link( &net_a, (u_char*)packet, hdr->caplen ))==-1);
        c = packet_send_link( &net_a, (u_char*)packet, hdr->caplen );
        if(c < hdr->caplen){
            printf("error: send len too short!\n");
        }
        /*
        if(hdr->caplen>1000){
            int i;
            struct timeval start,end;  
            gettimeofday(&start, NULL );
            for(i=0;i<10000;++i){
                libnet_write_link( net_a, (u_char*)packet, hdr->caplen );
            }
            gettimeofday(&end, NULL );
            long timeuse=1000000 * ( end.tv_sec - start.tv_sec ) + (end.tv_usec - start.tv_usec);
            long timeper=timeuse/10000;
            printf("time per packet: %ld\n",timeper); 
        }
        */
        //printf("A write: %d\n", c );
    }
}

void* dev_a_capture(void *arg)
{
    //dev_a = pcap_lookupdev( errbuf );
    dev_a="eth0";

    if ( dev_a == NULL)
    {
        printf("pcap_lookupdev: %s ", errbuf );
        exit( 0 );
    }
    printf("get dev: '%s'  \n", dev_a );

    pt_a = pcap_open_live( dev_a, 65535, 1, 1, errbuf );
    if( pt_a == NULL )
    {
        printf("pcap_open_live:%s ", errbuf );
        exit(0);
    }

    pcap_loop( pt_a, -1, dev_a_handle, NULL);
}

void dev_b_handle( u_char *devId, const struct pcap_pkthdr *hdr, const u_char *packet )
{
    //printf("%s,capture size :%d  ",devId, hdr->caplen );

    u_int8_t eth_a[ETH_ALEN];
    u_int8_t eth_b[ETH_ALEN];

    struct ether_header ehdr;
    memcpy( &ehdr, packet, sizeof( struct ether_header ));

    /* Only transmit source address is 221(eth2 MAC last bytes)  */
    if( ehdr.ether_dhost[ETH_ALEN-1] == 0xce    ){
        //printf("TO  dst: %X:%X:%X:%X:%X:%X, src: %X\n", ehdr.ether_dhost[0],ehdr.ether_dhost[1],
            //ehdr.ether_dhost[2],ehdr.ether_dhost[3],ehdr.ether_dhost[4],
            //ehdr.ether_dhost[ETH_ALEN-1], ehdr.ether_shost[ETH_ALEN-1] );
        return;
    }
    if( ehdr.ether_shost[ETH_ALEN-1] == 0xce )
    {
        //printf("B  src: %X:%X:%X:%X:%X:%X, dst:%X\n", ehdr.ether_shost[0],ehdr.ether_shost[1],
            //ehdr.ether_shost[2],ehdr.ether_shost[3],ehdr.ether_shost[4],
            //ehdr.ether_shost[ETH_ALEN-1], ehdr.ether_dhost[ETH_ALEN-1] );
        int c;
       //while((c = libnet_write_link( net_b, (u_char*)packet, hdr->caplen ))==-1);
       c = packet_send_link( &net_b, (u_char*)packet, hdr->caplen );
       //printf("B write: %d ", c );
    }
}

void* dev_b_capture(void *arg)
{
    //dev_b = pcap_lookupdev( errbuf );
    dev_b = "eth1";

    pt_b = pcap_open_live( dev_b, 65535, 1, 1, errbuf );
    if( pt_b == NULL )
    {
        printf("pcap_open_live:%s ", errbuf );
        exit(0);
    }

    pcap_loop( pt_b, -1, dev_b_handle, NULL);
}

int main( int argc, char* argv[] )
{
    /* regise signal ctrl+c stop capture */
    signal( SIGINT, sigproc ) ;

    int cpu_num=sysconf(_SC_NPROCESSORS_CONF);
    printf("cpu num: %d\n",cpu_num);
    /* init libnet */
    net_a.type=PACKET_TYPE_LINK;
    if(packet_send_init( &net_a, "eth1", errbuf )==-1)
    {
        fprintf(stderr, "libnet_init fail:%s \n", errbuf );
        return 0;
    }

    net_b.type=PACKET_TYPE_LINK;
    if(packet_send_init( &net_b, "eth0", errbuf )==-1)
    {
        fprintf(stderr, "libnet_init fail:%s ", errbuf );
        return 0;
    }

    /* create thread */
    int status;
    printf("create a ");
    status = pthread_create( &pid_a, NULL, dev_a_capture, NULL );
    if ( status != 0 )
    {
        printf( "pthread_create( A ) faile. ");
        goto end;
    }
    printf("create b ");
    status = pthread_create( &pid_b, NULL, dev_b_capture, NULL );
    if ( status != 0 )
    {
        printf( "pthread_create( B ) faile. ");
        goto end;
    }
    pthread_join( pid_a, NULL );
    pthread_join( pid_b, NULL );
    /*
    cpu_set_t mask1;
    cpu_set_t mask2;
    CPU_ZERO(&mask1);
    CPU_ZERO(&mask2);
    CPU_SET(4, &mask1);CPU_SET(5, &mask1);
    CPU_SET(6, &mask2);CPU_SET(7, &mask2);
    if (pthread_setaffinity_np(pid_a, sizeof(mask1), &mask1) < 0) {
        fprintf(stderr, "set thread affinity failed\n");
    }
    if (pthread_setaffinity_np(pid_b, sizeof(mask2), &mask2) < 0) {
        fprintf(stderr, "set thread affinity failed\n");
    }
    */
end:
    pcap_close( pt_a );
    pcap_close( pt_b );
    return 0;
}
/** 
 * read_netdev_proc - read net dev names form proc/net/dev 
 * @devname: where to store dev names, devname[num][len] 
 */  
/*
static int read_netdev_proc(void *devname, const int num, const int len)  
{  
    FILE *fh;  
    char buf[512];  
        int cnt = 0;  
        char *dev = (char *)devname;  
  
        if(devname == NULL || num < 1 || len < 4){  
            printf("read_netdev_proc: para error\n");  
            return -1;  
        }  
  
        memset(devname, 0, len * num);  
  
    fh = fopen(_PATH_PROCNET_DEV, "r");  
    if (!fh) {  
        fprintf(stderr, "Warning: cannot open %s (%s). Limited output.\n",  
            _PATH_PROCNET_DEV, strerror(errno));   
        return -1;  
      }  
  
    fgets(buf, sizeof buf, fh);
    fgets(buf, sizeof buf, fh);  
  
        cnt = 0;  
    while (fgets(buf, sizeof buf, fh) && cnt < num) {  
            char *s, name[IFNAMSIZ];  
            s = get_name(name, buf);      
  
            strncpy(devname, name, len);  
            devname += len;  
            printf("get_name: %s\n", name);  
    }  
  
    if (ferror(fh)) {  
            perror(_PATH_PROCNET_DEV);  
    }  
  
    fclose(fh);  
    return 0;  
}
*/
